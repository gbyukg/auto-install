#!/usr/bin/env bash

echo '========================== R31 upgrade install hotfix ==================='
set -x

db2 connect to DB_50
db2 "CREATE TABLE TRACKER_TRACKER_QUERIES (ID INTEGER NOT NULL, MONITOR_ID CHAR(36), QUERY_ID CHAR(36), DATE_MODIFIED TIMESTAMP)"
