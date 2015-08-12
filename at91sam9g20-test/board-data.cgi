#!/usr/bin/env bash

sqlite3 ~/data/sensors.db << EOT
select "Content-type: text/html

<html>
	<header>
		<style>
			table.dl {
				width:100%;
				border-collapse:collapse;
			}
			table.dl tr:nth-child(odd) {
				background: #ccc;
			}
		</style>
	</header>
	<body>
		<table>
			<tr>
				<td valign='top'>
					<table border='1' class='dl'>";
						select "
						<caption><h1>"||name||" (Board) Information</h1></caption>
						<tr><td><b>Board Key</b></td><td>"||key||"</td></tr>
						<tr><td><b>Board Display Name</b></td><td>"||name||"</td></tr>
						<tr><td><b>Board Description</b></td><td>"||descr||"</td></tr>
						<tr><td><b>Board Status</b></td><td>"||status||"</td></tr>
						<tr><td><b>Board Registration Date/Time</b></td><td>"||time||"</td></tr>
						<tr><td><b>Type</b></td><td>"||type||"</td></tr>"
						from keys where key="$1";
					select "
					</table>
					<table border='1' class='dl'>
						<caption><h3>Member Of</h3></caption>";
						select "
						<tr><td><a href=group-data.cgi?"||groups.parent||">"||keys.name||"</a></td></tr>"
						from keys join groups where keys.key=groups.parent and groups.child="$1";
					select "
					</table>
					<table border='1' class='dl'>
						<caption><h3>Configuration</h3></caption>";
						select "
						<tr><td><b>"||name||"</b></td><td>"||value||"</td></tr>"
						from "$1.config";
					select "
					</table>
					<iframe src=http://maps.google.com/maps?q="||
						(select value from "$1.config" where name="latitude")||","||
						(select value from "$1.config" where name="longitude")||
						"&z=15&output=embed style='width:100%;height:70%'></iframe>
				</td>
				<td>";
					select "
					<table border='1' class='dl'>
						<caption><h3>Sensor Data</h3></caption>
						<tr>
							<th>Count</th>
							<th>Date/Time</th>
							<th>Device ID</th>
							<th>Value</th>
							<th>Type</th>
						</tr>";
						select "
						<tr>
							<th>"||rowid||"</th>
							<td>"||time||"</td>
							<td>"||devid||"</td>
							<td>"||value||"</td>
							<td>"||type||"</td>
						</tr>" from "$1.data" order by rowid desc limit 32;
					select "
					</table>
				</td>
			</tr>
		</table>
	</body>
</html>";
EOT

