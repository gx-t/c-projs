#!/bin/sh

LED="board0.led0"
TERM="board0.term0"
PULSE="board0.counter-input"
COUNTER="board0.counter"
PERIOD=1


init() {
	echo gpio $LED 7 enable output 0
		echo gpio $PULSE 3 enable output 0
		echo ds18b20 $TERM 4 presense
		echo counter $COUNTER init
}

prepare() {
	echo gpio $LED 7 1
		echo gpio $PULSE 3 1
		echo ds18b20 $TERM 4 convert
}

collect() {
	echo gpio $LED 7 0
		echo gpio $PULSE 3 0
		echo '.begin transaction;'
		echo ds18b20 $TERM 4 read
		echo counter $COUNTER read
		echo '.end transaction;'
}

send() {
	echo ".dump outbox"
}

delete() {
	echo "delete from outbox;"
}

init | ./test
while true
do
	prepare | ./test
	sleep $PERIOD
	collect | ./test | curl --upload-file - http://shah32768.sdf.org/cgi-bin/sql-test.cgi
	sleep $PERIOD
done


#echo "create table outbox (id integer primary key, time timestamp, devid text, value float);"
#echo "select time,value,devid from outbox where devid='board0.counter';" | sqlite3 sensors.db
#echo "select time,value,devid from outbox where id between 10 and 15;" | sqlite3 sensors.db
#delete from outbox where id between 0 and 35000
#select * from outbox where id between (select min("id") from outbox)  and (select min("id")+2 from outbox);
#select * from outbox where id between (select max("id")-16 from outbox) and (select max("id") from outbox) and devid="board0.term0";
#select id,datetime(time, 'unixepoch'),devid,value from outbox where id between (select max(id)-32 from outbox) and (select max(id) from outbox);
