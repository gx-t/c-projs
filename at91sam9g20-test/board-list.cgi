#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
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
				<th>Board Key</th>
				<th>Board Display Name</th>
				<th>Board Description</th>
				<th>Board Status</th>
				<th>Board Registration Date/Time</th>
				<th>Type</th>
				<th>View Data</th>
			</tr>";
			select "
			<tr>
				<td>"||key||"</td>
				<td>"||name||"</td>
				<td>"||descr||"</td>
				<td>"||status||"</td>
				<td>"||time||"</td>
				<td>"||type||"</td>
				<td><a href=board-data.cgi?"||key||">...</a></td>
			</tr>" from keys where type="board" order by type desc, time desc, name;
		select "
		</table>
	</body>
</html>";
EOT

