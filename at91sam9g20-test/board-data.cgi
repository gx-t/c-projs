#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
select "Content-type: text/html

<html>
	<header>
		<style>
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
			<td valign='top'>
				<table border='1' class='dtable'>
					<caption><h3>Board Information</h3></caption>";
					select "
					<tr><td><b>Board Key</b></td><td>"||key||"</td></tr>
					<tr><td><b>Board Display Name</b></td><td>"||name||"</td></tr>
					<tr><td><b>Board Description</b></td><td>"||descr||"</td></tr>
					<tr><td><b>Board Status</b></td><td>"||status||"</td></tr>
					<tr><td><b>Board Registration Date/Time</b></td><td>"||time||"</td></tr>"
					from keys where key="$1";
				select "
				</table>
				<table border='1' class='dtable'>
					<caption><h3>Board Configuration</h3></caption>";
					select "
					<tr><td><b>"||name||"</b></td><td>"||value||"</td></tr>"
					from "$1.config";
				select "
				</table>
			</td>
			<td>
				<table border='1' class='dtable'>
					<caption><h3>Board Sensor Data</h3></caption>
					<tr>
						<th>Count</th>
						<th>Date/Time</th>
						<th>Device ID</th>
						<th>Value</th>
					</tr>";
					select "
					<tr>
						<th>"||rowid||"</th>
						<td>"||time||"</td>
						<td>"||devid||"</td>
						<td>"||value||"</td>
					</tr>" from "$1.data" where rowid between (select max(rowid)-16 from "$1.data") and (select max(rowid) from "$1.data");
				select "
				</table>
			</td>
		</tr>
	</body>
</html>";
EOT

