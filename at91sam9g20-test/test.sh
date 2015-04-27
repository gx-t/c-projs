#!/bin/sh

echo ds18b20 4 presense
while true
do
	echo gpio board0.led0 7 1
	echo ds18b20 board0.term0 4 convert
	sleep 1
	echo gpio board0.led0 7 0
	echo ds18b20 board0.term0 4 read
	sleep 1
done | ./test shell

