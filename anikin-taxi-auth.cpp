/*
*    anikin-taxi-auth.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the authenticator for the taxi service
*/

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main()
{
    /*
     *      This process is run from ws-script instead of this:
     *
     *          cat >>/tmp/fifos/from_client
     *
     *          as this:
     *
     *          ./anikin-taxi-auth
     *
     *      It reads the stdin line by line and with each line does the following things
     *
     *      1.  -> {"event":"auth","data":{"auth_string":"somestring"}}
     *          Creates the file /tmp/fifos/auth_strings/$somestring
     *          Does system("tail -f /tmp/fifos/auth_strings/$somestring </dev/null&");
     *          This call will just write everything that's in this file back to current client
     *          Note: </dev/null is important - it prevents tail from using stdin that the current
     *          program needs to get messages from a client
     *
     *      2.  -> {"event":"personal_message","data":{"auth_string":"somestring" ... the rest of the message}}
     *          Writes this line to /tmp/fifos/auth_strings/$somestring (which will be delivered to
     *          an authenticated client through tail -f that's already running after successful auth
     *
     *      3.  -> any other line
     *          Writes it to /tmp/fifos/from_client (to be processed by a marketplace broadcast server)
     */
    
    bool is_authenticated = false;
    std::string input_string;
    std::ofstream ofs_from_client("/tmp/fifos/from_client",
                                  std::ios::binary | std::ios::app | std::ios::ate | std::ios::out);
    while (true)
    {
        if (!getline(std::cin, input_string))
        {
            usleep(10000);
            continue;
        }
        
        std::cerr << "anikin-taxi-auth(" << getpid() << "): READ STRING: " << input_string << std::endl;
        
        // 1.   Check for an auth string
        char auth_pattern[] = "{\"event\":\"auth\",\"data\":{\"auth_string\":\"";
        auto i = input_string.find(auth_pattern);
        if (i != std::string::npos)
        {
            auto j = input_string.find("\"", i + sizeof(auth_pattern) - 1);
            if (j != std::string::npos)
            {
                std::string auth_string(input_string, i + sizeof(auth_pattern) - 1, j - i - sizeof(auth_pattern) + 1);
                std::cerr << "anikin-taxi-auth(" << getpid() << "): AUTH: got an auth string: " << auth_string << std::endl;
                
                // Check that auth_string is alphanum or '-' and no more than 64 chars
                bool is_ok = true;
                for (auto c : auth_string)
                    if (!(c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '-'))
                    {
                        is_ok = false;
                        break;
                    }
                if (!is_ok || auth_string.size() > 64)
                {
                    std::cerr << "anikin-taxi-auth(" << getpid() << "): auth string is invalid format" << std::endl;
                    continue;
                }
                
                // Don't allow to authenticate twice through one connection
                // Because it will creat two tail -f's which is a mess :-)
                if (is_authenticated)
                {
                    std::cerr << "anikin-taxi-auth(" << getpid() << "): a seccond attempt to authenticate. Blocked. Do it via a different connection please :-)" << std::endl;
                    continue;
                }
                
                // Create the file /tmp/fifos/auth_strings/$somestring
                auth_string = "/tmp/fifos/auth_strings/" + auth_string;
                std::cerr << "anikin-taxi-auth(" << getpid() << "): creating a file " << auth_string << std::endl;
                int fd = open(auth_string.c_str(), O_RDWR | O_CREAT, 0666);
                fchmod(fd, 0666);
                close(fd);
                std::cerr << "anikin-taxi-auth(" << getpid() << "): created (or not :-) ) a file, errno: " << strerror(errno) << std::endl;
                
                is_authenticated = true;
                
                // And start tailing it
                // Note: the resuls will go to the stdout
                auth_string = "tail -f " + auth_string + " </dev/null&";
                std::cerr << "anikin-taxi-auth(" << getpid() << "): staring this: " << auth_string << std::endl;
                if (system(auth_string.c_str()) != 0)
                {
                    std::cerr << "anikin-taxi-auth(" << getpid() << "): system failed, errno=" << strerror(errno) << std::endl;
                }
                
                // From this moment and on everything written to /tmp/fifos/auth_strings/$somestring
                // will go directly to the client
                
                continue;
            }
        }
        
        // 2.   Check for the personal message
        char personal_message_pattern[] = "{\"event\":\"personal_message\",\"data\":{\"auth_string\":\"";
        i = input_string.find(auth_pattern);
        if (i != std::string::npos)
        {
            auto j = input_string.find("\"", i + sizeof(personal_message_pattern) - 1);
            if (j != std::string::npos)
            {
                std::string auth_string(input_string, i + sizeof(personal_message_pattern) - 1,
                                        j - i - sizeof(personal_message_pattern) + 1);
                std::cerr << "anikin-taxi-auth: PERSONAL MESSAGE got an auth string: " << auth_string << std::endl;
                
                // Check that auth_string is alphanum or '-' and no more than 64 chars
                bool is_ok = true;
                for (auto c : auth_string)
                    if (!(c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '-'))
                    {
                        is_ok = false;
                        break;
                    }
                if (!is_ok || auth_string.size() > 64)
                {
                    std::cerr << "anikin-taxi-auth(" << getpid() << "): PERSONAL MESSAGE auth string is invalid format" << std::endl;
                    continue;
                }
                
                // Open the file /tmp/fifos/auth_strings/$somestring
                auth_string = "/tmp/fifos/auth_strings/" + auth_string;
                std::cerr << "anikin-taxi-auth(" << getpid() << "): opening a file " << auth_string << std::endl;
                int fd = open(auth_string.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
                std::cerr << "anikin-taxi-auth(" << getpid() << "): opened (or not :-) ) a file, errno: " << strerror(errno) << ", fd=" << fd
                    << std::endl;

                // Write input_string to the file and close the file
                input_string.push_back('\n');
                if (write(fd, input_string.c_str(), input_string.size()) < input_string.size())
                {
                    std::cerr << "anikin-taxi-auth(" << getpid() << "): write failed, errno=" << strerror(errno) << std::endl;
                }
                close(fd);
                
                
                continue;
            }
        }
        
        // 3.   Any other string just write to a broadcast server input: /tmp/fifos/from_client
        ofs_from_client << input_string << std::endl;
    }
    
	return 0;
}
