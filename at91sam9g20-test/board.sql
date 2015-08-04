--Table templates
CREATE TABLE "22265df2-1e31-4f8f-a778-53d724c8ff9c.config" (name text, value text, unique(name) on conflict replace);
CREATE TABLE "22265df2-1e31-4f8f-a778-53d724c8ff9c.data" (time timestamp, devid text, value float, key text);
