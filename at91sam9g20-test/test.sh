#!/bin/sh

LED="board0.led0"
TERM="board0.term0"
PULSE="board0.counter-input"
COUNTER="board0.counter"
PERIOD=1

(echo gpio $LED 7 enable output 0
echo gpio $PULSE 3 enable output 0
echo ds18b20 $TERM 4 presense
echo counter $COUNTER init
while true
do
	echo gpio $LED 7 1
	echo gpio $PULSE 3 1
	echo ds18b20 $TERM 4 convert
	sleep $PERIOD
	echo gpio $LED 7 0
	echo gpio $PULSE 3 0
	echo ds18b20 $TERM 4 read
	echo counter $COUNTER read
	sleep $PERIOD
done) | ./test shell

