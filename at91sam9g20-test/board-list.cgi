#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
.separator " "
select "Content-type: text/html

<html>
<header>
<style>
tr:nth-child(even) {
	background: #CCC;
}
td {
	text-align: center;
}
</style>
</header>
<body>
<table border='1'>
<caption><h3>List of Registered Boards</h3></caption>
<tr>
	<td><b>Board Key</b></td>
	<td><b>Board Display Name</b></td>
	<td><b>Board Description</b></td>
	<td><b>Board Status</b></td>
	<td><b>Board Registration Date/Time</b></td>
	<td><b>View Config</b></td>
	<td><b>View Data</b></td>
</tr>";
select "<tr>
<td>"||key||"</td>
<td>"||name||"</td>
<td>"||descr||"</td>
<td>"||status||"</td>
<td>"||time||"</td>
<td><a href=view-config.cgi?"||key||">...</a></td>
<td><a href=view-data.cgi?"||key||">...</a></td>
</tr>" from keys;
select "</table>
</body>
</html>";
EOT

