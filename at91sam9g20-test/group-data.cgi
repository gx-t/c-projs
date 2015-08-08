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
						<caption><h1>"||name||" (Group) Information</h1></caption>
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
							<th>Type</th>
							<th>View Data</th>
						</tr>";
						select "
						<tr>
							<td>"||keys.key||"</td>
							<th>"||keys.name||"</th>
							<td>"||keys.descr||"</td>
							<td>"||keys.status||"</td>
							<td>"||keys.time||"</td>
							<td>"||keys.type||"</td>
							<td><a href=board-data.cgi?"||keys.key||">...</a></td>
						</tr>" from keys join groups where keys.type='board' and keys.key=groups.child and groups.parent="$1";
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
							<th>Type</th>
							<th>View Data</th>
						</tr>";
						select "
						<tr>
							<td>"||keys.key||"</td>
							<th>"||keys.name||"</th>
							<td>"||keys.descr||"</td>
							<td>"||keys.status||"</td>
							<td>"||keys.time||"</td>
							<td>"||keys.type||"</td>
							<td><a href=group-data.cgi?"||keys.key||">...</a></td>
						</tr>" from keys join groups where keys.type='group' and keys.key=groups.child and groups.parent="$1";
					select "
					</table>
				</td>
			</tr>
		</table>
	</body>
</html>";
EOT

