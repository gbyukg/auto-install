<?php

$config = array(

    // DB settings
    'db' => array(
        'type' => 'db2', // mysql or db2
        'host' => 'localhost',
        'port' => '50000',
        'username' => 'gbyukg',
        'password' => 'admin',
        'name' => 'sugarcrm',
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
        'directory' => '/document/gbyukg/www/sugar/pull_install_18519',
    ),

);
