#!/usr/bin/env bash
sqlite3 -batch ~/data/sensors.db 2> /dev/null
echo -e -n "Content-type: text/plain\n\nOK"
