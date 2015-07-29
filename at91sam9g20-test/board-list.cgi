#!/usr/bin/env bash

sqlite3 sensors.db << EOT
.separator " "
select "Content-type: text/html

<html>
<header>
<style>
tr:nth-child(even) {
	background: #CCC
}
</style>
</header>
<body>
<table border='1' style='width:100%'>
<tr>
	<td>Board Key</td>
	<td>Board Display Name</td>
	<td>Board Description</td>
	<td>Board Status</td>
	<td>Board Registration Date/Time</td>
	<td>View Config</td>
	<td>View Data</td>
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

