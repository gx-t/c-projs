wget -t 0 -c http://www.busybox.net/downloads/binaries/latest/busybox-armv5l
copy to /bin
ln -s /bin/busybox-armv5l /usr/bin/awk
.mode insert
select time,devid,value from outbox
====
Error in board datasheet - I2C SCL and SDA are swapped

26.06.2015
TODO:
1. Server DB name - data
2. gzip send, receive (php)
3. send key=.... (board key)
Config:
Table: config
Columns: key TEXT, name TEXT, value TEXT
Params: board, 
Sensors:
devid, type
