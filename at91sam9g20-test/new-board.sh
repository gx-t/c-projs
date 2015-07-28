#!/usr/bin/env bash

echo -n 'Display name: '
read board
echo -n 'Description: '
read descr
key=`uuidgen`
config="$key.config"
data="$key.data"

sqlite3 sensors.db << EOT
begin transaction;

create table "keys" (key text, name text, descr text, status text, time timestamp, unique(key) on conflict abort);
insert into keys values("$key", "$board", "$descr", "enabled", CURRENT_TIMESTAMP);

create table "$config" (name text, value text);
insert into "$config" values("measure-period", "1");
insert into "$config" values("send-period", "10");
insert into "$config" values("data-cgi", "http://shah32768.sdf.org/cgi-bin/sensor-data.cgi");
insert into "$config" values("config-cgi", "http://shah32768.sdf.org/cgi-bin/sensor-data.cgi");

create table "$data" (time TIMESTAMP, devid text, value real, key text);

end transaction;
EOT
echo $key

