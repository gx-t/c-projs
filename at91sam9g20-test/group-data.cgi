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

			table.dd {
				width:100%;
				border-collapse:collapse;
			}
			table.dd tr:nth-child(odd) {
				background: #ccc;
			}
			table.dd td {
				text-align: center;
			}
		</style>
	</header>
	<body>
		<table>
			<tr>
				<td valign='top'>
					<table border='1' class='dl'>";
						select "
						<caption><h1>"||name||" Information</h1></caption>
						<tr><td><b>Board Key</b></td><td>"||key||"</td></tr>
						<tr><td><b>Board Display Name</b></td><td>"||name||"</td></tr>
						<tr><td><b>Board Description</b></td><td>"||descr||"</td></tr>
						<tr><td><b>Board Status</b></td><td>"||status||"</td></tr>
						<tr><td><b>Board Registration Date/Time</b></td><td>"||time||"</td></tr>
						<tr><td><b>Group</b></td><td>"||parent||"</td></tr>
						<tr><td><b>Type</b></td><td>"||type||"</td></tr>"
						from keys where key="$1";
					select "
					</table>
				</td>
				<td>";
					select "
					<table border='1' class='dd'>
						<caption><h3>Boards</h3></caption>
						<tr>
							<th>Key</th>
							<th>Name</th>
							<th>Description</th>
							<th>Status</th>
							<th>Registration Date/Time</th>
							<th>Parent Group</th>
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
							<td>"||parent||"</td>
							<td>"||type||"</td>
							<td><a href=board-data.cgi?"||key||">...</a></td>
						</tr>" from keys where parent="$1" and type="board" order by time desc;
					select "
					</table>
					<table border='1' class='dd'>
						<caption><h3>Groups</h3></caption>
						<tr>
							<th>Key</th>
							<th>Name</th>
							<th>Description</th>
							<th>Status</th>
							<th>Registration Date/Time</th>
							<th>Parent Group</th>
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
							<td>"||parent||"</td>
							<td>"||type||"</td>
							<td><a href=group-data.cgi?"||key||">...</a></td>
						</tr>" from keys where parent="$1" and type="group" order by time desc;
					select "
					</table>
				</td>
			</tr>
		</table>
	</body>
</html>";
EOT

