wget -t 0 -c http://www.busybox.net/downloads/binaries/latest/busybox-armv5l
copy to /bin
ln -s /bin/busybox-armv5l /usr/bin/awk
.mode insert
select time,devid,value from outbox
====
Error in board datasheet - I2C SCL and SDA are swapped
