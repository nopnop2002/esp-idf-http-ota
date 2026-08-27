#!/bin/bash
#sudo apt install mosquitto-clients
BROKER="broker.emqx.io"
TOPIC="/topic/test"
COUNTER=0
INTERVAL=5

while true
do
	PAYLOAD="Hello World "${COUNTER}
	echo ${PAYLOAD}
	mosquitto_pub -h ${BROKER} -t ${TOPIC} -m "${PAYLOAD}"
	echo mosquitto_pub -h ${BROKER} -t ${TOPIC} -m ${PAYLOAD}
	COUNTER=`expr $COUNTER + 1`
	#echo ${COUNTER}
	sleep ${INTERVAL}
done
