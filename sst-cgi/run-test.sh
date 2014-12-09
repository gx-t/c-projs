#!/usr/bin/env bash

echo "dump_values" | ./sst.cgi
##
echo "echo-line
aaa bbb ccc" | ./sst.cgi
##
echo "register
abrakadabra
Gugush
Gugushyan
gugush@gugushyan.com" | ./sst.cgi
##
echo "register
kaa" | ./sst.cgi

