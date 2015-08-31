#!/usr/bin/env bash

DBNAME=$1
: ${DBNAME:=sugarult}
export DB2DBDFT=$DBNAME # set $DBNAME as the default branch
#
# db2stop # stop DB2
db2start # start DB2
# db2 "FORCE APPLICATION ALL" # flush all connections
[[ "Xtrue" == X"${DROP_DB}" ]] && \
    db2 connect to "${DBNAME}" && \
    db2 quiesce database immediate force connections && \
    db2 unquiesce database && \
    db2 connect reset && \
    db2 deactivate db "${DBNAME}" && \
    db2 "DROP DATABASE $DBNAME" # drop the previously existing database if it exists

db2 "CREATE DATABASE $DBNAME USING CODESET UTF-8 TERRITORY US COLLATE USING UCA500R1_LEN_S2 PAGESIZE 32 K" # create the database from scratch and enable case-insensitive collation
db2 "CONNECT TO $DBNAME" # make a connection to update the parameters below
db2 "UPDATE database configuration for $DBNAME using applheapsz 32768 app_ctl_heap_sz 8192"
db2 "UPDATE database configuration for $DBNAME using stmtheap 60000"
db2 "UPDATE database configuration for $DBNAME using locklist 50000"
db2 "UPDATE database configuration for $DBNAME using indexrec RESTART"
db2 "UPDATE database configuration for $DBNAME using logfilsiz 1000"
db2 "UPDATE database configuration for $DBNAME using logprimary 12"
db2 "UPDATE database configuration for $DBNAME using logsecond 30"
db2 "UPDATE database configuration for $DBNAME using DATABASE_MEMORY AUTOMATIC" #Prevent memory exceeding
db2set DB2_COMPATIBILITY_VECTOR=4008
db2 "CREATE BUFFERPOOL SUGARBP IMMEDIATE  SIZE 1000 AUTOMATIC PAGESIZE 32 K"
db2 "CREATE  LARGE  TABLESPACE SUGARTS PAGESIZE 32 K  MANAGED BY AUTOMATIC STORAGE EXTENTSIZE 32 OVERHEAD 10.5 PREFETCHSIZE 32 TRANSFERRATE 0.14 BUFFERPOOL SUGARBP"
db2 "CREATE USER TEMPORARY TABLESPACE SUGARXGTTTS IN DATABASE PARTITION GROUP IBMDEFAULTGROUP PAGESIZE 32K MANAGED BY AUTOMATIC STORAGE EXTENTSIZE 32 PREFETCHSIZE 32 BUFFERPOOL SUGARBP OVERHEAD 7.5 TRANSFERRATE 0.06 NO FILE SYSTEM CACHING"
