#!/usr/bin/env bash
echo -e "Content-type: text/plain\n\n"
sqlite3 -batch ~/data/sensors.db 2> /dev/null

