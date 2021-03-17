#!/bin/bash

curl "http://$1:1234/auth/sendsms?phone=123"
curl "http://$1:1234/auth/by_smscode?phone=123&smscode=7938"

