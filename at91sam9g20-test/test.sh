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
	echo '.begin transaction;'
	echo ds18b20 $TERM 4 read
	echo counter $COUNTER read
	echo '.end transaction;'
	sleep $PERIOD
done) | ./test shell | sqlite3 sensors.db


#echo "create table sensors (id INTEGER PRIMARY KEY, time TIMESTAMP, board TEXT, device TEXT, value TEXT);"
#echo "select time,value,device from sensors where device='board0.counter';" | sqlite3 sensors.db
#echo "select time,value,device from sensors where id between 10 and 15;" | sqlite3 sensors.db
