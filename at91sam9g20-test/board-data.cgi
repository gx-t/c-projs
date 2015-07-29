#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
.separator " "
select "Content-type: text/html

<html>
<header>
<style>
td {
	text-align: center;
}
.dtable {
	width:100%;
	border-collapse:collapse;
}
.dtable tr:nth-child(odd) {
	background: #ccc;
}
</style>
</header>
<body>
<table>
<tr>
<td>
<table border='1' class='dtable'>
<caption><h3>Board Information</h3></caption>";
select "
<tr><td><b>Board Key</b></td><td>"||key||"</td></tr>
<tr><td><b>Board Display Name</b></td><td>"||name||"</td></tr>
<tr><td><b>Board Description</b></td><td>"||descr||"</td></tr>
<tr><td><b>Board Status</b></td><td>"||status||"</td></tr>
<tr><td><b>Board Registration Date/Time</b></td><td>"||time||"</td></tr>"
from keys where key="$1";
select "</table>
</td>
<td rowspan='2'>
<table border='1' class='dtable'>
<caption><h3>Board Sensor Data</h3></caption>
<tr>
	<th>Date/Time</th>
	<th>Device ID</th>
	<th>Value</th>
</tr>";
select "<tr>
<td>"||time||"</td>
<td>"||devid||"</td>
<td>"||value||"</td>
</tr>" from "$1.data";
select "</table>
</td>
</tr>
<tr>
<td>
<table border='1' class='dtable'>
<caption><h3>Board Configuration</h3></caption>";
select "
<tr><td><b>"||name||"</b></td><td>"||value||"</td></tr>"
from "$1.config";
select "</table>
</td>
</tr>
</body>
</html>";
EOT

