#!/bin/bash

killall websocketd
killall anikin-taxi-http

mkdir /tmp/fifos/
echo >/tmp/fifos/to_client
echo >/tmp/fifos/from_client

sudo ./websocketd bash ./ws-script.sh >ws_agents.log 2>1&

tail -n 1 -q -f /tmp/fifos/from_client | ./anikin-taxi-http ws-agent >/tmp/fifos/to_client 2>mpl.log&

# Moscow
./test_riders_drivers drivers 55.57157 37.5379743 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 55.57157 37.5379743 1000 1000 30 >>/tmp/fifos/from_client&

#Syktyvkar
./test_riders_drivers drivers 61.6689353 50.8002925 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 61.6689353 50.8002925 1000 1000 30 >>/tmp/fifos/from_client&

#Makhachkala
./test_riders_drivers drivers 42.9691002 47.493167 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 42.9691002 47.493167 1000 1000 30 >>/tmp/fifos/from_client&

#Simpheropol
./test_riders_drivers drivers 44.9532761 34.1019158 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 44.9532761 34.1019158 1000 1000 30 >>/tmp/fifos/from_client&

#Minsk
./test_riders_drivers drivers 53.8937038 27.5575191 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 53.8937038 27.5575191 1000 1000 30 >>/tmp/fifos/from_client&

#Gaborone
./test_riders_drivers drivers -24.6407154 25.8962554 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders -24.6407154 25.8962554 1000 1000 30 >>/tmp/fifos/from_client&

#Dakar
./test_riders_drivers drivers 14.712695 -17.4617723 1000 1000 30 >>/tmp/fifos/from_client&
./test_riders_drivers riders 14.712695 -17.4617723 1000 1000 30 >>/tmp/fifos/from_client&


# Pompano beach - 1000 cars - 10s per new car
./test_riders_drivers drivers 26.2752915 -80.1084394 10000 1000 1000 >>/tmp/fifos/from_client&
./test_riders_drivers riders 26.2752915 -80.1084394 10000 10000 1000 >>/tmp/fifos/from_client&


# Moscow - city center - 10 cars
./test_riders_drivers drivers 55.7570202 37.615977 100 100 10 >>/tmp/fifos/from_client&
./test_riders_drivers riders 55.7570202 37.615977 100 100 10 >>/tmp/fifos/from_client&

#Gaborone - 10 cars
./test_riders_drivers drivers -24.6407154 25.8962554 100 100 10 >>/tmp/fifos/from_client&
./test_riders_drivers riders -24.6407154 25.8962554 100 100 10 >>/tmp/fifos/from_client&


#Gaborone - 1000 cars - 10s per new car
./test_riders_drivers drivers -24.6407154 25.8962554 9000 1000 1000 >>/tmp/fifos/from_client&
./test_riders_drivers riders -24.6407154 25.8962554 10000 10000 1000 >>/tmp/fifos/from_client&

#Makhachkala - 1000 cars - 10s per new car
./test_riders_drivers drivers 42.9691002 47.493167 9000 1000 1000 >>/tmp/fifos/from_client&
./test_riders_drivers riders 42.9691002 47.493167 10000 10000 1000 >>/tmp/fifos/from_client&

#Dakar - 1000 cars - 10s per new car
./test_riders_drivers drivers 14.712695 -17.4617723 9000 1000 1000 >>/tmp/fifos/from_client&
./test_riders_drivers riders 14.712695 -17.4617723 10000 10000 1000 >>/tmp/fifos/from_client&






# Moscow - near Butovo
./test_riders_drivers drivers 55.57157 37.5379743 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 55.57157 37.5379743 100 100 300 >>/tmp/fifos/from_client&

#Syktyvkar
./test_riders_drivers drivers 61.6689353 50.8002925 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 61.6689353 50.8002925 100 100 300 >>/tmp/fifos/from_client&

#Makhachkala
./test_riders_drivers drivers 42.9691002 47.493167 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 42.9691002 47.493167 100 100 300 >>/tmp/fifos/from_client&

#Simpheropol
./test_riders_drivers drivers 44.9532761 34.1019158 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 44.9532761 34.1019158 100 100 300 >>/tmp/fifos/from_client&

#Minsk
./test_riders_drivers drivers 53.8937038 27.5575191 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 53.8937038 27.5575191 100 100 300 >>/tmp/fifos/from_client&

#Gaborone
./test_riders_drivers drivers -24.6407154 25.8962554 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders -24.6407154 25.8962554 100 100 300 >>/tmp/fifos/from_client&

#Dakar
./test_riders_drivers drivers 14.712695 -17.4617723 100 100 300 >>/tmp/fifos/from_client&
./test_riders_drivers riders 14.712695 -17.4617723 100 100 300 >>/tmp/fifos/from_client&
