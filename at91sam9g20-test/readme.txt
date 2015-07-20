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

===BOARD DATABASE===

PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE config (name text, value text, unique(name) on conflict replace);
INSERT INTO "config" VALUES('board','Second test board (prefix - board1)');
INSERT INTO "config" VALUES('send-period','10');
INSERT INTO "config" VALUES('measure-period','1');
INSERT INTO "config" VALUES('description','Second board multiline
description is here');
INSERT INTO "config" VALUES('key','9a5a9f7d-efe5-447f-9894-06fb91750ba6');
INSERT INTO "config" VALUES('data-cgi','http://www.seismoinstruments.am/insert.php');
INSERT INTO "config" VALUES('config-cgi','http://www.seismoinstruments.am/insert.php');
COMMIT;

2 tables: key.data (as "9a5a9f7d-efe5-447f-9894-06fb91750ba6.data") and key.config (as "9a5a9f7d-efe5-447f-9894-06fb91750ba6.config")
tables must not have id primary key to use simple .dump to avoid using awk, etc.
Board initialli sends key.config to server.
key.data has: time TIMESTAMP, devid TEXT, value FLOAT
key.config has: name TEXT, value TEXT
key.config *must* contain the following names:
"board" - board name
"key" - board key, used as prefix for table name and as key for web API call
"data-url" - server URL for SQL insertions
"config-url" - server URL for config insertions
"measure-period" - measurement period in seconds
"send-period" - send period in seconds
===
Data sends are done using simple .dump so they contain "CREATE TABLE..." that will be ignored by server SQL.
Data must be sent gzipped to lower the traffic
===
Server must response to sensor data with "OK" in case of success and with description in case of error (???)
Server response may also contain URL with board update script (or native executable)
Board requests must have the following format: <server php>?key=<key> Example:
http://seismo.firewall.am/insert.php?key="9a5a9f7d-efe5-447f-9894-06fb91750ba6"
Server checks if the key is registered key, if no - ignores the request.
Key is also used on server to identify the user that the board belongs to.



