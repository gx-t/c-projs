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
			<caption><h3>List of Groups</h3></caption>
			<tr>
				<th>Group Key</th>
				<th>Group Display Name</th>
				<th>Group Description</th>
				<th>Group Status</th>
				<th>Group Registration Date/Time</th>
				<th>Parent Group</th>
				<th>Type</th>
				<th>Child Nodes</th>
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
			</tr>" from keys where type="group" order by type desc, time desc, name;
		select "
		</table>
	</body>
</html>";
EOT

