#!/usr/bin/env bash

echo -n 'Display name: '
read board
echo -n 'Description: '
read descr
echo -n 'Latitude: '
read latitude
echo -n 'Longitude: '
read longitude
key=`uuidgen`
config="$key.config"
data="$key.data"

sqlite3 -batch sensors.db << EOT
begin transaction;

create table "keys" (key text, name text, descr text, status text, time timestamp, parent text, unique(key) on conflict abort);
insert into keys values("$key", "$board", "$descr", "enabled", CURRENT_TIMESTAMP, "00000000-0000-0000-0000-000000000000");

create table "$config" (name text, value text, unique(name) on conflict replace);
insert into "$config" values("measure-period", "1");
insert into "$config" values("send-period", "10");
insert into "$config" values("latitude", "$latitude");
insert into "$config" values("longitude", "$longitude");
insert into "$config" values("data-cgi", "http://shah32768.sdf.org/cgi-bin/board-send-data.cgi");
insert into "$config" values("config-cgi", "http://shah32768.sdf.org/cgi-bin/board-send-data.cgi");

create table "$data" (time TIMESTAMP, devid text, value real, key text);
select "$key";
end transaction;
EOT

