--Table templates
CREATE TABLE "keys" (key text, name text, descr text, status text, time timestamp, parent text, unique(key) on conflict abort);
CREATE TABLE "22265df2-1e31-4f8f-a778-53d724c8ff9c.config" (name text, value text, unique(name) on conflict replace);
CREATE TABLE "22265df2-1e31-4f8f-a778-53d724c8ff9c.data" (time TIMESTAMP, devid text, value real, key text);
CREATE TABLE "c893b811-352e-4667-b740-0d6ed91d0f95.config" (name text, value text, unique(name) on conflict replace);
CREATE TABLE "c893b811-352e-4667-b740-0d6ed91d0f95.data" (time TIMESTAMP, devid text, value real, key text);
