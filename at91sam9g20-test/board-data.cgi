#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
select "Content-type: text/html

<html>
	<header>
		<style>
			table.dd {
				width:100%;
				border-collapse:collapse;
			}
			table.dd tr:nth-child(odd) {
				background: #ccc;
			}
		</style>
	</header>
	<body>
		<table>
			<tr>
				<td valign='top'>
					<table border='1' class='dd'>
						<caption><h3>Board Information</h3></caption>";
						select "
						<tr><td><b>Board Key</b></td><td>"||key||"</td></tr>
						<tr><td><b>Board Display Name</b></td><td>"||name||"</td></tr>
						<tr><td><b>Board Description</b></td><td>"||descr||"</td></tr>
						<tr><td><b>Board Status</b></td><td>"||status||"</td></tr>
						<tr><td><b>Board Registration Date/Time</b></td><td>"||time||"</td></tr>
						<tr><td><b>Group</b></td><td>"||parent||"</td></tr>"
						from keys where key="$1";
					select "
					</table>
					<table border='1' class='dd'>
						<caption><h3>Board Configuration</h3></caption>";
						select "
						<tr><td><b>"||name||"</b></td><td>"||value||"</td></tr>"
						from "$1.config";
					select "
					</table>
					<iframe src=http://maps.google.com/maps?q=40.1920773,44.5099267&z=15&output=embed style='width:100%;height:70%'></iframe>
				</td>
				<td>";
					select "
					<table border='1' class='dd'>
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
						</tr>" from "$1.data" where rowid between (select max(rowid)-32 from "$1.data") and (select max(rowid) from "$1.data");
					select "
					</table>
				</td>
			</tr>
		</table>
	</body>
</html>";
EOT

