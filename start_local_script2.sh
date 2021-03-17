#!/bin/bash

curl "http://$1:1234/rider/reg?token=$2"
curl "http://$1:1234/marketplace/price_view?token=$2&A_lon=1.1&A_lat=2.2&A_addr=123&B_lon=3.3&B_lat=4.4&B_addr=567"

