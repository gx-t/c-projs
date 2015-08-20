----Table templates
create table "keys" (key text, name text, descr text, status text, time timestamp, type text, unique(key) on conflict abort)
create table groups (parent text, child text, unique(parent, child) on conflict abort)
create table data (time timestamp, devid text, value real, key text, type text)
create table config (name text, value text, key text,  unique(name, key) on conflict replace)
