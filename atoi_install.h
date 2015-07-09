//
//  atoi_install.h
//  newInstall
//
//  Created by 张贺 on 15-5-28.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#ifndef __atoi_install__
#define __atoi_install__

#include "atoi_git.h"
#include "atoi_curl.h"
#include "common.h"
#include <yajl/yajl_tree.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

char *get_cur_branch_name(void);

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

static void
pre_install(void);

static void
package_upgrade(const char *package_path);

void
start_install(void);

// install from pull request
void pull_install(void);

// install from branch
void branch_install(void);

void package_install(const char *packageInfo);

static long curl_install_step(const char *post);

static void
after_install(void);

// install from current git repository
void git_install(void);
#endif /* defined(__atoi_install__) */
