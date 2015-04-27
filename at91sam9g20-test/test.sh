#!/bin/sh

LED="board0.led0"
TERM="board0.term0"
PERIOD=1
echo ds18b20 $TERM 4 presense
while true
do
	echo gpio $LED 7 1
	echo ds18b20 $TERM 4 convert
	sleep $PERIOD
	echo gpio $LED 7 0
	echo ds18b20 $TERM 4 read
	sleep $PERIOD
done | ./test shell

