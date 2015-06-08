#!/usr/bin/env bash
echo "Content-type: text/html"
printf "\n\n"

echo "<html><body><table border='1' style='width:100%'>"
(echo ".header on"
echo ".mode html"
echo "select id,datetime(time, 'unixepoch'),devid,value from data where id between (select max(id)-32 from data) and (select max(id) from data);") | sqlite3 ~/data/sensors.db
echo "</table></body></html>"
