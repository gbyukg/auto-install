#!/usr/bin/env bash

clean_file_dir=$(date +%Y%m%d)
[[ -z "${1}" ]] || clean_file_dir="${1}"

clean_dir=/home/stallman/.ai_del/"${clean_file_dir}"
web_dir=/home/stallman/www
cache_dir=/home/stallman/tmp


echo "Drop instances in [${clean_file_dir}]"

[[ ! -d "${clean_dir}" ]] && echo "Done" && exit 0

drop_db()
{
    db_name=${1}
    printf "Drop database %s ...\n" "${db_name}"
    db2 connect to "${db_name}" && \
    db2 quiesce database immediate force connections && \
    db2 unquiesce database && \
    db2 connect reset && \
    db2 deactivate db "${db_name}" && \
    db2 "DROP DATABASE ${db_name}" # drop the previously existing database if it exists
}

for del_file in ${clean_dir}; do
    printf "\nRemoving %s ...\n" "${del_file}"
    rm -rf "${web_dir}/${del_file}" > /dev/null 2>&1
    drop_db "DB_${del_file}"
    rm -rf "${cache_dir}/${del_file}" > /dev/null 2>&1
done

rm -rf "${clean_dir}"
