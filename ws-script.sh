#/!bin/bash

# Everything received from /tmp/fifos/to_client will be written back to client
tail -n 1 -f /tmp/fifos/to_client </dev/null&

## Everything reeceived from a client will be redirected to /tmp/fifos/from_client_{PID}
## We need different file for a different client because "cat" remembers only its own
## current position in a file and it rewrites what other concurrent cats do
##cat >>/tmp/fifos/from_client

# 1. -> {"event":"auth","data":{"auth_string":"somestring"}} 
#	Result: creates /tmp/fifos/auth_strings/$somestring
#		does tail -f /tmp/fifos/auth_strings/$somestring </dev/null&
#		Why? Everything written to /tmp/fifos/auth_strings/$somestring will go to THIS client
# 2. -> {"event":"personal_message","data":{"auth_string":"somestring" ... the rest of the message}}
#	Result: writes this to /tmp/fifos/auth_strings/$somestring
#		Why? It goes directly to the client authenticated by somestring via tail -f above
# 3. -> anything else
#	Result: writes it to >>/tmp/fifos/from_client which goes to the broadcast server

./anikin-taxi-auth 2>>anikin-taxi-auth.log

