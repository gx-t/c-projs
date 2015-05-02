#!/usr/bin/env bash
printf "\n\n"

while read line
do
	echo "$line"
done | sqlite3 ~/data/sensors.db
echo "OK"

