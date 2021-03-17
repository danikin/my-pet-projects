#!/bin/bash

curl "http://$1:1234/rider/move_A?token=$2&flow_id=$3&lat=10.10&lon=15.15"
curl "http://$1:1234/rider/move_B?token=$2&flow_id=$3&lat=10.10&lon=15.15"

