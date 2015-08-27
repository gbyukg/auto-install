#!/usr/bin/env bash

#export ATOI_GIT_PATH="/home/stallman/mango/"
#export ATOI_WEB_PATH="/home/stallman/www/"
#export ATOI_BUILD_PATH="/home/stallman/mango/build/rome/builds/"
#export ATOI_INSTALL_HOOK_SCRIPT="/home/stallman/auto-install/install_hook.sh"
#export ATOI_INIT_DB_SCRIPT="/home/stallman/auto-install/initdb.sh"
#export ATOI_DEF_BASE_USER="sugareps"
#export ATOI_DEF_HEAD_USER="gbyukg"

#export ATOI_SC_LICENSE="5f41c8f0ed136c4ced2de01622f942f3"
#export ATOI_WEB_HOST="http://localhost:8888"
#export ATOI_SC_ADMIN="admin"
#export ATOI_SC_ADMIN_PWD="asdf"

#export DB_NAME="sugarcrm"
#export DB_PORT="50000"
#export DB_HOST="localhost"
#export DB_USER="stallman"
#export DB_PASSWORD="btit@ibm"

#export ATOI_FTS_TYPE="Elastic"
#export ATOI_FTS_HOST="localhost"
#export ATOI_FTS_PORT="9200"

#export ATOI_TMP_PATH="/home/stallman/tmp/"
export ATOI_CONFIG_PATH=/home/stallman/auto-install/config.ini

#gcc -Wall -g -lyajl -lgit2 common.c dictionary.c iniparser.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-sc -DSERVERINSTALL && ./atoi-sc install --branch-install sugareps:ibm_r30 --debug --install-name pull_install_18519 --no-dl
gcc -Wall -g  -lcurl -lyajl -I/usr/local/src/include -L/usr/local/src/lib -lgit2 src/common.c src/dictionary.c src/iniparser.c src/atoi_curl.c src/atoi_install.c src/atoi_git.c src/main.c -o atoi-sc -DSERVERINSTALL

#./atoi-sc install --pack-install "http://sc2.gnuhub.com/sugarsync/ibm_production/SugarUltimate-production-135-08102015.zip http://sc2.gnuhub.com/sugarsync/ibm_r31/SugarUltimate-r31-13-base-production-135-08132015.zip" --debug --install-name 50 --dbname DB_50

#gcc -Wall -g -lconfig -lyajl -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-sc -DSERVERINSTALL && ./atoi-sc install --web-install pull_install_18519 --debug --install-name pull_install_18519 --no-init-db
