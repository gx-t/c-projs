#!/usr/bin/env bash

echo -n 'Display name: '
read board
echo -n 'Description (^D to finish): '
read descr
key=`uuidgen`
config="$key.config"
data="$key.data"

sqlite3 sensors.db << EOT
begin transaction;
create table "$config" (name text, value text);
insert into "$config" values("key", "$key");
insert into "$config" values("board", "$board");
insert into "$config" values("descr", "$descr");
insert into "$config" values("registered", CURRENT_TIMESTAMP);
insert into "$config" values("status", "enabled");
insert into "$config" values("data-cgi", "http://shah32768.sdf.org/cgi-bin/sensor-data.cgi");
insert into "$config" values("config-cgi", "http://shah32768.sdf.org/cgi-bin/sensor-data.cgi");

create table "$data" (time TIMESTAMP, devid text, value real, key text);
end transaction;
EOT
echo $key

