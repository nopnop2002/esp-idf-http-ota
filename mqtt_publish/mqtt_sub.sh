#!/bin/bash
#sudo apt install mosquitto-clients moreutils
BROKER="broker.emqx.io"
TOPIC="/topic/test"
mosquitto_sub -h ${BROKER} -t ${TOPIC} -v | ts "%Y/%m/%d %H:%M:%S"
