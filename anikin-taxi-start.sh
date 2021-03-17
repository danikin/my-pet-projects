#!/bin/bash

echo Starting headqurters
echo It has http-server that faces end users and fifo to communicate with ws agents
./anikin-taxi-http headquarters mysqlx://root:De3695511@127.0.0.1 1234 >>headquarters.log 2>&1&

echo Starting parent process for ws agents
echo Each ws agent comminucate with its own end user
./websocketd ./anikin-taxi-http ws-agent >>ws_agents.log 2>1&

