#!/usr/bin/env bash

#  install_hook.sh
#  newInstall
#
#  Created by 张贺 on 15-6-27.
#  Copyright (c) 2015年 张贺. All rights reserved.

#!/usr/bin/env bash

star="********************"

[[ "debug" == "${1}"  ]] && readonly DEBUG=debug && set -x
shift

# before_install
readonly FUNC=${1}
shift

# pull_install_XXXX
readonly INSTALL_NAME=${1}
shift

# /document/gbyukg/www/Mango
readonly GIT_DIR=${1}
shift

# /document/gbyukg/www/sugar/
readonly WEB_DIR=${1}
shift

# /document/gbyukg/www/sugar/build_path/
readonly BUILD_DIR=${1}
shift

# /Users/gbyukg/tmp/
readonly TMP_DIR=${1}
shift

readonly WORK_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cus_echo()
{
    echo ""
    echo " $(tput setaf 5)${star} ${1} ${star}$(tput sgr0)"
}

exit_err()
{
    echo ""
    echo "$(tput setaf 1)Error: ${1}$(tput sgr0)"
    exit 1
}

error_check_exit()
{
    [[ 0 -ne "$?" ]] && exit_err "${1}"
}

#install_pro()
#{
    ##cd "${GIT_DIR}"
    #cd /document/gbyukg/www/test/test
    #echo "github token:"
    #read token
    #sha=$(echo ${token} | git hash-object -w --stdin)
    #echo "sha: ${sha}"

    ## write token into the config file
#}

before_build()
{
    cus_echo "Running hook [before_build]"
    cd "${GIT_DIR}"
    cus_echo "Running hook [before_build] finished"
}

after_build()
{
    cus_echo "Running hook [after_build]"
    [[ -a "${WEB_DIR}${INSTALL_NAME}" ]] &&
    cus_echo "Directory [${WEB_DIR}${INSTALL_NAME}] exists, removing..." &&
    rm -rf "${WEB_DIR}${INSTALL_NAME}"

    cus_echo "Copying from [${BUILD_DIR}ult/sugarcrm] => [${WEB_DIR}${INSTALL_NAME}]"
    cp -r "${BUILD_DIR}ult/sugarcrm" "${WEB_DIR}${INSTALL_NAME}"

    cus_echo "Running hook [after_build] finished"
}

build_code()
{
    before_build

    cus_echo "Running hook [build_code]"

    cd "${GIT_DIR}"/build/rome
    php build.php -clean -cleanCache -flav=ult -ver='7.1.5' -dir=sugarcrm -build_dir="${BUILD_DIR}"

    error_check_exit "Build code failure"
    cus_echo "Running hook [build_code] finished" &&
        after_build
}

before_init_db()
{
    cus_echo "Running hook [before_init_db]"

    cus_echo "Running hook [before_init_db] finished"
}

after_init_db()
{
    cus_echo "Running hook [before_init_db]"

    cus_echo "Running hook [before_init_db] finished"
}

before_fetch_code()
{
    cus_echo "Running hook [before_fetch_code]"
}

after_fetch_code()
{

    cus_echo "Running hook [after_fetch_code]"
}

package_install()
{
    cus_echo "Running hook [package_install]"
    local install_meth="${1}"
    local download_url="${2}"
    local package_name="${3}"
    local tmp_dir="${4}"
    #exit 1

    if [[ "Xurl" == X"${install_meth}" ]]; then # 安装文件存在于远程服务器
        #set -e
        cus_echo "Download package [${download_url}]"
        [[ -f "${TMP_DIR}${package_name}" ]] && rm -rf "${TMP_DIR}${package_name}"
        [[ -f "${TMP_DIR}${tmp_dir}" ]] && rm -rf "${TMP_DIR}${tmp_dir}"
        # 下载压缩包
        cus_echo "下载文件到 ${TMP_DIR} ..."
        wget -P "${TMP_DIR}" "${download_url}"
        error_check_exit "无法下载压缩包 [${download_url}]"

        # 解压缩文件
        cus_echo "解压缩文件 [${TMP_DIR}${package_name}] 到 [${TMP_DIR}${tmp_dir}] 下..."
        unzip -d "${TMP_DIR}${tmp_dir}" "${TMP_DIR}${package_name}" > /dev/null 2>&1
        error_check_exit "解压文件 [${TMP_DIR}${package_name}] 失败"

        local install_package_name=$(ls -d "${TMP_DIR}${tmp_dir}"/SugarUlt-Full*)

        echo "${install_package_name}"
        [[ -d "${WEB_DIR}/${INSTALL_NAME}" ]] \
            && cus_echo "安装文件 ${INSTALL_NAME} 已经存在, 删除..." \
            && rm -rf "${WEB_DIR}/${INSTALL_NAME}"

        cus_echo "移动文件 [${install_package_name}] 到 ${WEB_DIR}/${INSTALL_NAME} 下..."
        mv "${install_package_name}" "${WEB_DIR}/${INSTALL_NAME}"

        #cus_echo "清空缓存目录 ${TMP_DIR}${tmp_dir}..."
        #rm -rf "${TMP_DIR}${tmp_dir}"

    elif [[ "Xlczip" == X"${install_meth}" ]]; then # 安装文件存在于本地
        # 判断文件是否存在
        [[ -f "${package_name}" ]] || error_check_exit "安装文件 [${package_name}] 不存在!"

        # 解压缩文件
        cus_echo "解压缩文件 [${TMP_DIR}${package_name}] 到 [${tmp_dir}] 下..."
        unzip -d "${TMP_DIR}${tmp_dir}" "${package_name}" > /dev/null 2>&1
        error_check_exit "解压文件 [${package_name}] 失败"

        # 解压后安装包名
        local install_package_name=$(ls -d "${TMP_DIR}${tmp_dir}"/SugarUlt-Full*)

        [[ -d "${WEB_DIR}/${INSTALL_NAME}" ]] \
            && cus_echo "安装文件 ${INSTALL_NAME} 已经存在, 删除..." \
            && rm -rf "${WEB_DIR}/${INSTALL_NAME}"

        cus_echo "移动文件 [${install_package_name}] 到 ${WEB_DIR}/${INSTALL_NAME} 下..."
        mv "${install_package_name}" "${WEB_DIR}/${INSTALL_NAME}"
        [[ -f "${TMP_DIR}${tmp_dir}" ]] && rm -rf "${TMP_DIR}${tmp_dir}"

        # 删除临时文件
        #cus_echo "清空缓存目录 ${TMP_DIR}${tmp_dir}..."
        #rm -rf "${TMP_DIR}${tmp_dir}"

    elif [[ "Xupgrade" == X"${install_meth}" ]]; then
        mkdir -p "${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/custom/include/Config/configs"

    cat <<SYSCONFIG > system.config.php
<?php
\$config['apache_binary'] = '/document/gbyukg/soft/apache2.2/bin/apachectl';
\$config['apache_user'] = 'gbyukg';
\$config['apache_group'] = 'admin';
SYSCONFIG

    cat <<db2CONFIG > db2cli.config.php
<?php
\$config['db2profile'] = '/Users/gbyukg/sqllib/db2profile';
\$config['db2createscript'] = '/document/gbyukg/www/test/newInstall/newInstall/initdb.sh';
\$config['db2runas'] = '';
db2CONFIG

    cat <<logCONFIG > logger.config.php
<?php
\$config['log_file'] = '${TMP_DIR}SugarInstanceManager.log';
\$config['log_dir'] = '${TMP_DIR}';
logCONFIG

        sed -i '' "s/\s*\$su->backup();//g" "${WEB_DIR}/${INSTALL_NAME}"/vendor/sugareps/SugarInstanceManager/upgrade.php;

        #[[ -f "${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/custom/include/Config/configs/system.config.php" ]] &&
        local sysconf="${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/custom/include/Config/configs/system.config.php"
        local db2conf="${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/custom/include/Config/configs/db2cli.config.php"
        local logconf="${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/custom/include/Config/configs/logger.config.php"

        [[ -n "${DEBUG}" ]] && cat "${sysconf}" && cat "${db2conf}" && cat "${logconf}"

        [[ -f "${sysconfig}" ]] && rm -rf "${sysconf}"
        [[ -f "${db2conf}" ]] && rm -rf "${db2conf}"
        [[ -f "${logconf}" ]] && rm -rf "${logconf}"
        mv system.config.php "${sysconf}"
        mv db2cli.config.php "${db2conf}"
        mv logger.config.php "${logconf}"

        cd "${WEB_DIR}/${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/"

        cus_echo "安装升级包文件 [${package_name}]..."
        php upgrade.php --instance_path "${WEB_DIR}${INSTALL_NAME}" --upgrade_zip "${package_name}" 
    fi
}

after_package_install()
{
    cus_echo "Running hook [after_package_install]"

    cus_echo "Running hook [after_package_install] finished"
}

before_install()
{
    # 修改 PHP 错误日志
    cus_echo "Running hook [before_install]"

    cd "${WEB_DIR}${INSTALL_NAME}"
    cus_echo "composer install..."
    composer install
    cus_echo "Running hook [before_install] finished"
}

install()
{
    before_install
    #cp -r /document/gbyukg/www/sugar/vendor /document/gbyukg/www/sugar/pull_install_17744/vendo

    local tmp_file="${1}"
    local install_url="${2}"
    local setup_license_key="${3}"
    local setup_fts_type="${4}"
    local setup_fts_host="${5}"
    local setup_fts_port="${6}"
    local setup_db_database_name="${7}"
    local setup_db_port_num="${8}"
    local setup_db_host_name="${9}"
    local setup_db_admin_user_name="${10}"
    local setup_db_admin_password="${11}"
    local setup_site_admin_user_name="${12}"
    local setup_site_admin_password="${13}"

    #cus_echo "web dir: ${WEB_DIR}"
    #cus_echo "git dir: ${GIT_DIR}"
    #cus_echo "build:   ${BUILD_DIR}"
    #cus_echo "in name: ${INSTALL_NAME}"
    #echo "install_url:       ${install_url}"
    #echo "setup_license_key: ${setup_license_key}"
    #echo "set_up_fts_type:   ${setup_fts_type}"
    #echo "setup_fts_host:    ${setup_fts_host}"
    #echo "setup_fts_port:    ${setup_fts_port}"
    #echo "db name            ${setup_db_database_name}"
    #echo "db port            ${setup_db_port_num}"
    #echo "db host            ${setup_db_host_name}"
    #echo "db admin           ${setup_db_admin_user_name}"
    #echo "db pwd             ${setup_db_admin_password}"
    #echo "install user       ${setup_site_admin_user_name}"
    #echo "install pwd        ${setup_site_admin_password}"
    #cus_echo "${tmp_file}"
    local step0="current_step=0&goto=Next&language=en_us&instance_url=${install_url}/install.php"
    local step1="current_step=1&goto=Next"
    local step_to_pdf="checkInstallSystem=true&to_pdf=1&sugar_body_only=1"
    local step2="current_step=2&goto=Next&setup_license_accept=on"
    local step3="current_step=3&goto=Next&install_type=custom&setup_license_key=${setup_license_key}"
    local step4="current_step=4&goto=Next&setup_db_type=ibm_db2"
    local checkdb="checkDBSettings=true\
&to_pdf=1\
&sugar_body_only=1\
&setup_fts_type=${setup_fts_type}\
&setup_fts_host=${setup_fts_host}\
&setup_fts_port=${setup_fts_port}\
&demoData=no\
&setup_db_database_name=${setup_db_database_name}\
&setup_db_port_num=${setup_db_port_num}\
&setup_db_host_name=${setup_db_host_name}\
&setup_db_admin_user_name=${setup_db_admin_user_name}\
&setup_db_admin_password=${setup_db_admin_password}"
    local step5="current_step=5\
&goto=Next\
&setup_db_drop_tables=\
&setup_db_create_sugarsales_user=\
&demoData=no\
&setup_fts_type=${setup_fts_type}\
&setup_fts_host=${setup_fts_host}\
&setup_fts_port=${setup_fts_port}\
&setup_db_database_name=${setup_db_database_name}\
&setup_db_port_num=${setup_db_port_num}\
&setup_db_host_name=${setup_db_host_name}\
&setup_db_admin_user_name=${setup_db_admin_user_name}\
&setup_db_admin_password=${setup_db_admin_password}\
&setup_db_admin_password_entry=${setup_db_admin_password}"
    local step6="current_step=6\
&goto=Next\
&setup_system_name=SugarCRM\
&setup_site_admin_user_name=${setup_site_admin_user_name}\
&setup_site_admin_password=${setup_site_admin_password}\
&setup_site_admin_password_retype=${setup_site_admin_password}\
&setup_site_url=${install_url}"
    local step7="current_step=7\
&goto=Next\
&setup_site_sugarbeet_anonymous_stats=yes\
&setup_site_sugarbeet_automatic_checks=yes\
&setup_site_session_path=\
&setup_site_log_dir=.\
&setup_site_guid="
    local step8="current_step=8&goto=Next"
    local step9="current_step=9&goto=Next"
    local step10="current_step=10&goto=Next&language=en_us&install_type=custom&default_user_name=${setup_site_admin_user_name}"
    local step11="default_user_name=${setup_site_admin_user_name}&next=Next"

    echo "${step0}" > "${tmp_file}"
    echo "${step1}" >> "${tmp_file}"
    echo "${step_to_pdf}" >> "${tmp_file}"
    echo "${step2}" >> "${tmp_file}"
    echo "${step3}" >> "${tmp_file}"
    echo "${step4}" >> "${tmp_file}"
    echo "${checkdb}" >> "${tmp_file}"
    echo "${step5}" >> "${tmp_file}"
    echo "${step6}" >> "${tmp_file}"
    echo "${step7}" >> "${tmp_file}"
    echo "${step8}" >> "${tmp_file}"
    echo "${step9}" >> "${tmp_file}"
    echo "${step10}" >> "${tmp_file}"
    echo "${step11}" >> "${tmp_file}"
}

dataloader()
{
    cus_echo "Running hook [dataloader]"

    local DB_LOCAL="${1}"
    local DB_PORT="${2}"
    local DB_NAME="${3}"
    local DB_USER="${4}"
    local DB_PWD="${5}"
    if [[ -z "${6}"  ]]; then
        local DATALOADER_PATH="${GIT_DIR}"ibm/dataloaders
    else
        local DATALOADER_PATH=${6}
    fi
    cus_echo "running data loader in [${DATALOADER_PATH}]..."
    cd "${DATALOADER_PATH}" && rm -rf config.php

    cat <<CONFIG > config.php
<?php

\$config = array(

    // DB settings
    'db' => array(
        'type' => 'db2', // mysql or db2
        'host' => '${DB_LOCAL}',
        'port' => '${DB_PORT}',
        'username' => '${DB_USER}',
        'password' => '${DB_PWD}',
        'name' => '${DB_NAME}',
    ),

    // default bean field/values used by Utils_Db::createInsert()
    'bean_fields' => array(
        'created_by' => '1',
        'date_entered' => '2012-01-01 00:00:00',
        'modified_user_id' => '1',
        'date_modified' => '2012-01-01 00:00:00',
    ),

    // sugarcrm
    'sugarcrm' => array(
        // full path of the installed sugarcrm instance
        'directory' => '${WEB_DIR}${INSTALL_NAME}',
    ),

);
CONFIG

#!!! Remember to run the FCH Manager on every client team or hierarchy change !!!
     #--> cd batch_sugar/RTC_19211; php -f rtc_19211_main.php RTC_19211 <--
    php populate_SmallDataset.php

    #[[ X"true" == X"${GIT_RESET}" ]] && cd "${GIT_DIR}ibm/dataloaders" && git checkout config.php
    [[ -z "${6}"  ]] && cd "${GIT_DIR}ibm/dataloaders" && git checkout config.php

    cd "${WEB_DIR}${INSTALL_NAME}/batch_sugar/RTC_19211"
    php -f rtc_19211_main.php


    cus_echo "Running hook [dataloader] finished"
}

# $2: install dir
after_install()
{
    touch "${WEB_DIR}${INSTALL_NAME}/sql.sql"
    cp "${WORK_DIR}/repair.sh" "${WEB_DIR}${INSTALL_NAME}"
    cp -f "${WORK_DIR}/ChromePhp.php" "${WEB_DIR}${INSTALL_NAME}/include/ChromePhp.php"
    echo "require_once 'ChromePhp.php';" >> "${WEB_DIR}${INSTALL_NAME}/include/utils.php"

    add_ignore
    mv install_gitignore "${WEB_DIR}${INSTALL_NAME}/.gitignore"

    add_project
    mv install_project "${WEB_DIR}${INSTALL_NAME}/.project"

    # format js files
    format_js

    [[ -f "${WEB_DIR}${INSTALL_NAME}/runQuickRepair.php" ]] ||
    cp "${WEB_DIR}${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/templates/scripts/php/runQuickRepair.php" \
        "${WEB_DIR}${INSTALL_NAME}/runQuickRepair.php"

    [[ -f "${WEB_DIR}${INSTALL_NAME}/runRebuildJSGroupings.php" ]] ||
    cp "${WEB_DIR}${INSTALL_NAME}/vendor/sugareps/SugarInstanceManager/templates/scripts/php/runRebuildJSGroupings.php" \
        "${WEB_DIR}${INSTALL_NAME}/runRebuildJSGroupings.php"

    cd "${WEB_DIR}${INSTALL_NAME}"
    cus_echo "Init git"
    git init
    {
        git add . && git commit -m "init"
    } >& /dev/null

    #cus_echo "生成TAGS..."
    #find . -name "*.php" -not -path "./cache/*" > ./.git/list && gtags -f ./.git/list

    cus_echo "Running hook [after_install] finished"
}

add_ignore()
{
    cat <<IGNORE > install_gitignore
.gitignore
*.sql
*.log
.*
notes
cache
create_tag.sh
repair.sh
tags
sidecar/minified
*.patch
config_override.php
GPATH
GRTAGS
GTAGS
*.exvim
IGNORE
}

format_js()
{
    # fomate js files
    type js-beautify >&/dev/null 2>&1
    [[ $? == 0 ]] && {
    cd "${WEB_DIR}${INSTALL_NAME}"
    pwd
    cus_echo "formatting JS files..."
        find cache/include/javascript sidecar/minified -type f -iname "*.js" -exec sh -c \
            'for i do js-beautify -r "${i}"; done' sh {} +
    }
}

add_project()
{
    cat <<PROJECT > install_project
<?xml version="1.0" encoding="UTF-8"?>
<projectDescription>
<name>${INSTALL_NAME}</name>
<comment></comment>
<projects>
</projects>
<buildSpec>
    <buildCommand>
        <name>org.eclipse.wst.validation.validationbuilder</name>
        <arguments>
        </arguments>
    </buildCommand>
    <buildCommand>
        <name>org.eclipse.dltk.core.scriptbuilder</name>
        <arguments>
        </arguments>
    </buildCommand>
</buildSpec>
<natures>
    <nature>org.eclipse.php.core.PHPNature</nature>
</natures>
</projectDescription>
PROJECT
}

run_ut()
{
    cus_echo "Running unittest..."
    cd "${WEB_DIR}${INSTALL_NAME}/tests"
    phpunit
}

load_avl()
{
    cus_echo "Importing AVL..."
    cd "${WEB_DIR}${INSTALL_NAME}/custom/cli"
    cus_echo "avl.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl.csv idlMode=true
    cus_echo "01-update.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/01-update.csv
    cus_echo "02-remap.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/02-remap.csv
    cus_echo "03-update.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/03-update.csv
    cus_echo "04-update.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/04-update.csv
    cus_echo "05-update.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/05-update.csv
    cus_echo "06-winplan.csv"
    php cli.php task=Avlimport file="${WEB_DIR}${INSTALL_NAME}"/custom/install/avl/06-winplan.csv
}

init()
{
    [[ -d "${TMP_DIR}" ]] || mkdir -p "${TMP_DIR}"
}

test()
{
    set -x
    cus_echo ""
}

umask 022

[[ -f ${ATOI_INSTALL_HOOK} ]] && source "${ATOI_INSTALL_HOOK}"

${FUNC} "$@"

