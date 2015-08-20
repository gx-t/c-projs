----Table templates
create table config (name text, value text, unique(name) on conflict replace)
create table data (time timestamp, devid text, value float, key text, type text)
