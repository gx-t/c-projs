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

#read board key
key=$(cat key)


#init sensors
echo "gpio 29 enable gpio 29 output gpio 29 1
gpio 31 enable gpio 31 output gpio 31 1
gpio 3 enable gpio 3 output gpio 3 0
counter init
gpio 3 1
ds18b20 4 presense" | ./test -q > /dev/null
echo 1 > /sys/class/misc/adc/ch0_enable

ss=`curl -s http://shah32768.sdf.org/cgi-bin/board-get-config.cgi?$key` &&
printf "%s" "$ss" > .config
unset ss
source .config
mkdir -p outbox

echo 'gpio 29 0 gpio 31 0 gpio 3 1' | ./test -q > /dev/null
usleep 200000

collect() {
	echo 'begin transaction;'
	for i in `seq 1 $send_period)`
		do
			dt=`date -u '+%Y-%m-%d %H:%M:%S'`
			echo "gpio 29 1 gpio 3 1 ds18b20 4 convert
				`sleep $measure_period`
				gpio 29 0 gpio 3 0
				insert into data values( '$dt' , '$THERM0' , ds18b20 4 read , '$key' , 'temp' ); 
				insert into data values( '$dt' , '$COUNTER' , counter read , '$key' , 'count' ); 
				insert into data values( '$dt' , '$THERM1' , lm75 0x4F read , '$key' , 'temp' ); 
				insert into data values( '$dt' , '$ADCLIGHT0' , `cat /sys/class/misc/adc/ch0_value`, '$key' , 'light' );"
		done
	echo 'commit;'
}
while :
do
	collect | ./test -q | gzip -fc > .tmp
	mv .tmp "outbox/$(date -u +%s)"
done &

echo "collect_pid=$!"

cd outbox
while sleep `expr $send_period \\* $measure_period`
do
	echo 'gpio 31 1' | ../test -q > /dev/null
	ls | while read ff
	do
		cat $ff | [[ `curl -s --upload-file - $data_cgi` == "OK" ]] && rm $ff
	done
	echo 'gpio 31 0' | ../test -q > /dev/null
done &

echo "send_pid=$!"
