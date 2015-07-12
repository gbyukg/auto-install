#!/usr/bin/env bash

export ATOI_GIT_PATH="/home/stallman/mango/"
export ATOI_WEB_PATH="/home/stallman/www/"
export ATOI_BUILD_PATH="/home/stallman/mango/build/rome/builds/"
export ATOI_INSTALL_HOOK_SCRIPT="/home/stallman/auto-install/install_hook.sh"
export ATOI_INIT_DB_SCRIPT="/home/stallman/auto-install/initdb.sh"
export ATOI_DEF_BASE_USER="sugareps"
export ATOI_DEF_HEAD_USER="gbyukg"

export ATOI_SC_LICENSE="5f41c8f0ed136c4ced2de01622f942f3"
export ATOI_WEB_HOST="http://localhost"
export ATOI_SC_ADMIN="admin"
export ATOI_SC_ADMIN_PWD="asdf"

export DB_NAME="sugarcrm"
export DB_PORT="50000"
export DB_HOST="localhost"
export DB_USER="stallman"
export DB_PASSWORD="111111"

export ATOI_FTS_TYPE="Elastic"
export ATOI_FTS_HOST="localhost"
export ATOI_FTS_PORT="9200"

export ATOI_TMP_PATH="/home/stallman/tmp/"

gcc -Wall -g -lconfig -lyajl -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-sc -DSERVERINSTALL && ./atoi-sc install --branch-install sugareps:ibm_r30 --debug --install-name pull_install_18519
