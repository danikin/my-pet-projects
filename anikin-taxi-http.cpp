#include <map>
#include <string>
#include <locale>
#include <iostream>
#include <thread>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "marketplace_api.h"
#include "marketplace_api_drivers.h"
#include "geo.h"
#include "http_interface.h"
#include "console_interface.h"
#include "ws_interface.h"
#include "auth.h"

int main(int argc, char *argv[])
{
    if (argc >= 2 && !strcmp(argv[1], "debug"))
    {
        // Debug marketplaces from standard input
        console_interface::debug_marketplaces();
    }
    else
    if (argc >= 2 && !strcmp(argv[1], "ws-agent"))
    {
        ws_interface::ws_agent ws_srv;

        std::cerr << "Starting the communication channel with headquarters" << std::endl;

        // Starts the headquarters fifo in a separate thread
        // Note: if thread is created as an unnamed objects then it crashes. Don't know why
        std::thread _t_([&](void){
            try {
                ws_srv.agent_vs_headquarters_loop();
            } catch (std::exception &e) {
                std::cerr << "Can't start the communication channel: " << e.what() << std::endl;
            }
        });

        std::cerr << "Starting agent_vs_client_loop" << std::endl;

        // Start a web socket interface
        ws_srv.agent_vs_client_loop();
    }
    else
    if (argc >= 4 && !strcmp(argv[1], "headquarters"))
    {
        try {
            std::cout << "Initializing geo database" << std::endl;
            
            // Initialize the address database
            geo::compact_addresses addr2;
            /*upload_streets_from_json(addr, "Street2.txt");
            upload_houses_from_json(addr, "House1.txt");
            upload_houses_from_json(addr, "House2.txt");

            // Note: POI MUST be uploaded after houses because they leverage houses
            // to insert into word maps
            upload_poi_from_json(addr, "POI.txt");

            upload_everything(addr, "central-fed-district-latest.osm.parsed3",
                              1000000000ull, 1000000000ull);
            upload_everything(addr, "botswana-latest.osm.parsed3",
                              2000000000ull, 2000000000ull);
            
            addr.print_stat();*/
            
            upload_everything2(addr2);
            
            std::cout << "Initializing multicastors" << std::endl;
            
            fifo_interface::multicastor rider_multicastor;
            fifo_interface::multicastor driver_multicastor;
            
            std::cout << "Initializing marketplace" << std::endl;

            // Initialize marketplace
            // TODO: it is used from different threads - sync is needed!
            marketplace::marketplace_api mrktplc(argv[2] /* mysql_url */,
                                                 rider_multicastor, driver_multicastor);

             std::cout << "Initializing riders" << std::endl;

            // Initialize riders
            marketplace::marketplace_api_riders riders;
            
            std::cout << "Initializing drivers" << std::endl;

            // Initialize drivers
            marketplace::marketplace_api_drivers drivers;
 
            std::cout << "Initializing auth" << std::endl;

            // Initialize auth
            // TODO: it is used from different threads - sync is needed!
            auth::authenticator ath;
            
            std::cout << "Starting the communication channel with ws agents" << std::endl;

            // Starts the headquarters fifo in a separate thread
            // Note: if thread is created as an unnamed objects then it crashes. Don't know why
            std::thread _t_([&](void){
                try {
                    ws_interface::headquarters_loop(ath, mrktplc, rider_multicastor, driver_multicastor);
                } catch (std::exception &e) {
                    std::cerr << "Can't start the communication channel: " << e.what() << std::endl;
                }
            });
  
            std::cout << "Starting http on the port " << argv[3] << std::endl;
            
            // Start an http interface
            http_interface::http_server http_srv;
            http_srv.run(atoi(argv[3] /* port */), ath, addr2, mrktplc, riders, drivers);
        } catch (std::exception &e) {
            std::cerr << "Can't start headquarters: " << e.what() << std::endl;
        }
    }
    else
        std::cout << "Usage\n\tanikin-taxi-http debug|ws-agent|headquarters mysql_url port\n";
    
    return 0;
}
