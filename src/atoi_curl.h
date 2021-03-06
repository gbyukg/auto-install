//
//  atoi_curl.h
//  newInstall
//
//  Created by 张贺 on 15-5-28.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#ifndef __atoi_install__curl__
#define __atoi_install__curl__

#include <curl/curl.h>
#include "common.h"
#include "atoi_git.h"

char curl_err_buf[CURL_ERROR_SIZE];

CURL *atoi_get_curl(const char *url,  struct curl_slist *headers);

int my_trace(CURL *handle, curl_infotype type,
            char *data, size_t size,
            void *userp);

int atoi_create_pull_request(void);

#endif /* defined(__atoi_install__curl__) */
