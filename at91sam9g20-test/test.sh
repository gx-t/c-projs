#!/bin/sh

#2 LEDS: pin 29, 31
#GENERATOR(PULSE) pin 3
#COUNTER pin 9
#DS18B20 pin 4
#LM75 pins 17,18

THERM0="therm-ds18b20"
THERM1="therm-lm75"
COUNTER="counter0"
ADCLIGHT0="adc-light0"

./test -q << EOT
gpio 29 enable gpio 29 output gpio 29 1
gpio 31 enable gpio 31 output gpio 31 1
gpio 3 enable gpio 3 output gpio 3 0
counter init
gpio 3 1
ds18b20 4 presense
EOT
echo 1 > /sys/class/misc/adc/ch0_enable

#read board key
key=$(cat key)
#configuration table name
config="$key.config"
#data table name
data="$key.data"
#ask server for board configuration
echo ".dump \"$config\"" | curl -s --upload-file - http://shah32768.sdf.org/cgi-bin/board-get-config.cgi | sqlite3 -batch sensors.db 2> /dev/null
echo "create table \"$data\" (time timestamp, devid text, value float, key text);" | sqlite3 -batch sensors.db 2> /dev/null

get_config() {
	echo "select value from \"$config\" where name=\"$1\";" | sqlite3 -batch sensors.db 
}

./test -q << EOT
gpio 29 0 gpio 31 0 gpio 3 1
EOT

sleep `get_config measure-period`

prepare() {
	./test -q << EOT
	gpio 29 1 gpio 31 0 gpio 3 1 ds18b20 4 convert
EOT
}

collect() {
	./test -q << EOT | sqlite3 -batch sensors.db
	gpio 29 0 gpio 31 1 gpio 3 0
	begin transaction;
		insert into "$data" values( CURRENT_TIMESTAMP , "$THERM0" , ds18b20 4 read , "$key" , "temp" ); 
		insert into "$data" values( CURRENT_TIMESTAMP , "$COUNTER" , counter read , "$key" , "count" ); 
		insert into "$data" values( CURRENT_TIMESTAMP , "$THERM1" , lm75 0x4F read , "$key" , "temp" ); 
		insert into "$data" values( CURRENT_TIMESTAMP , "$ADCLIGHT0" , `cat /sys/class/misc/adc/ch0_value`, "$key" , "light" ); 
	end transaction;
EOT
}

send() {
	echo "gpio 29 1	gpio 31 1" | ./test -q
	echo ".dump \"$data\"" | sqlite3 -batch sensors.db | gzip -fc | [[ `curl -s --upload-file - $(get_config data-cgi)` == "OK" ]]
}

delete() {
	echo "gpio 29 0 gpio 31 0" | ./test -q
	echo "delete from \"$data\";" | sqlite3 -batch sensors.db
}

cnt=0
while true
do
prepare
sleep `get_config measure-period`
collect
cnt=`expr $cnt + 1`
[[ $cnt == `get_config send-period` ]] && cnt=0 && send && delete
done

