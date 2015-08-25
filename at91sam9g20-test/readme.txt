===ON BOARD SIDE===
1. Get latest test.c, at91sam9g20.h, Makefile, test.sh - compile and put test and test.sh
2. Delete sensors.db - not needed at all
3. Create key file:
	cd /root
	echo -en "c893b811-352e-4667-b740-0d6ed91d0f95" > key

How it works:
1. Reads key from 'key' file
2. Asks server for configuration for given key
3. Saves the configuration to .config file
4. Sources the .config file
5. Separate background process scans sensors, puts the data to compressed files in outbox
6. An other background process scans outbox for files, sends and deletes sent file

===ON SERVER SIDE===
1. Create tables: config, data, groups, keys:
	CREATE TABLE config (name text, value text, key text,  unique(name, key) on conflict replace);
	CREATE TABLE data (time TIMESTAMP, devid text, value real, key text, type text);
	CREATE TABLE groups (parent text, child text, unique(parent, child) on conflict abort);
	CREATE TABLE keys (key text, name text, descr text, status text, time timestamp, type text, unique(key) on conflict abort);
2. Fill config:
	INSERT INTO config VALUES('measure_period','1','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('send_period','10','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('latitude','40.1920773','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('longitude','44.55','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('data_cgi','http://shah32768.sdf.org/cgi-bin/board-send-data.cgi','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('config_cgi','http://shah32768.sdf.org/cgi-bin/board-send-data.cgi','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO config VALUES('measure_period','1','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO config VALUES('send_period','10','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO config VALUES('latitude','40.1920773','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO config VALUES('longitude','44.5099267','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO config VALUES('data_cgi','http://shah32768.sdf.org/cgi-bin/board-send-data.cgi','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO config VALUES('config_cgi','http://shah32768.sdf.org/cgi-bin/board-send-data.cgi','22265df2-1e31-4f8f-a778-53d724c8ff9c');
3. Fill groups:
	INSERT INTO groups VALUES('00000000-0000-0000-0000-000000000000','6accc6d5-c6cd-4083-bd7b-7aeede51db21');
	INSERT INTO groups VALUES('6accc6d5-c6cd-4083-bd7b-7aeede51db21','0d2acecb-ecf2-49ba-824b-8870971d1925');
	INSERT INTO groups VALUES('6accc6d5-c6cd-4083-bd7b-7aeede51db21','abe67128-6f0c-44b5-99a4-f0b87d46cf46');
	INSERT INTO groups VALUES('6accc6d5-c6cd-4083-bd7b-7aeede51db21','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO groups VALUES('6accc6d5-c6cd-4083-bd7b-7aeede51db21','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO groups VALUES('abe67128-6f0c-44b5-99a4-f0b87d46cf46','c893b811-352e-4667-b740-0d6ed91d0f95');
	INSERT INTO groups VALUES('0d2acecb-ecf2-49ba-824b-8870971d1925','22265df2-1e31-4f8f-a778-53d724c8ff9c');
	INSERT INTO groups VALUES('00000000-0000-0000-0000-000000000000','04c2925b-039e-49d9-8d35-39788354a5f8');
	INSERT INTO groups VALUES('04c2925b-039e-49d9-8d35-39788354a5f8','1a566962-d7b4-4f6f-9360-8377773a24c6');
	INSERT INTO groups VALUES('04c2925b-039e-49d9-8d35-39788354a5f8','214bceae-d11c-4d25-a167-e9b0ec452c08');
	INSERT INTO groups VALUES('04c2925b-039e-49d9-8d35-39788354a5f8','0442baca-59cd-4db4-8370-42e51102f23e');
4. Fill keys:
	INSERT INTO keys VALUES('22265df2-1e31-4f8f-a778-53d724c8ff9c','Test Board 0','Description of Test Board 0, script generated','enabled','2015-07-30 20:29:21','board');
	INSERT INTO keys VALUES('c893b811-352e-4667-b740-0d6ed91d0f95','Test Board 1','Description of Test Board 1','enabled','2015-07-30 20:32:08','board');
	INSERT INTO keys VALUES('00000000-0000-0000-0000-000000000000','Root','Parent for All Groups and Devices','enabled','2015-08-05 11:36:09','group');
	INSERT INTO keys VALUES('6accc6d5-c6cd-4083-bd7b-7aeede51db21','Testing','Temporary Group Created for Testing','enabled','2015-08-08 16:56:40','group');
	INSERT INTO keys VALUES('0d2acecb-ecf2-49ba-824b-8870971d1925','Yerevan','Group of boards located in Yerevan and part of Testing group','enabled','2015-08-08 16:59:31','group');
	INSERT INTO keys VALUES('abe67128-6f0c-44b5-99a4-f0b87d46cf46','Gyumri','Group of boards located in Gyumri and part of Testing group','enabled','2015-08-08 17:00:38','group');
	INSERT INTO keys VALUES('04c2925b-039e-49d9-8d35-39788354a5f8','Spare Boards','Holder of spare boards - new boards that are not assigned yet or returned boards','disabled','2015-08-15 12:15:40','group');
	INSERT INTO keys VALUES('1a566962-d7b4-4f6f-9360-8377773a24c6','Spare Board','This board is not assigned yet','disabled','2015-08-15 12:22:25','board');
	INSERT INTO keys VALUES('214bceae-d11c-4d25-a167-e9b0ec452c08','Spare Board','This board is not assigned yet','disabled','2015-08-15 12:26:31','board');
	INSERT INTO keys VALUES('0442baca-59cd-4db4-8370-42e51102f23e','Spare Board','This board is not assigned yet','disabled','2015-08-15 12:28:12','board');

===MINIMUM PHP SUPPORT===

1. Create board list PHP with the following SQL statement:
	select * from keys where type="board" order by type desc, time desc, name;
2. Modify board data PHP to have the following SQL statement:
	select * from data where key="$1" order by rowid desc limit 128;

*For complete support see working CGIs and do the same in PHP:
	https://github.com/shah-/c-projs/tree/master/at91sam9g20-test
*Working example is here:
	http://shah32768.sdf.org/cgi-bin/board-list.cgi

