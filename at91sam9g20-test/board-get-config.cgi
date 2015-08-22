#!/usr/bin/env bash
echo -en "\n\n"
sqlite3 -batch ~/data/sensors.db 2> /dev/null << EOT
.separator =
select name, value from config where key="$1";
EOT
