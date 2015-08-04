//
//  atoi_git.h
//  newInstall
//
//  Created by 张贺 on 15-5-31.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#ifndef __atoi_git__
#define __atoi_git__

#include "common.h"
#include <git2.h>

#define git_err(func) {\
int error = 0;\
if ((error = func) < 0) {\
const git_error *e = giterr_last();\
extErr("libgit error %d/%d: %s\n", error, e->klass, e->message);\
}\
}

typedef struct _atoi_git_status_files
{
    uint count;
    char *path[10240];
} atoi_git_status_files;

typedef struct _atoi_git_status
{
    atoi_git_status_files c;    // Changes to be commited
    atoi_git_status_files a;    // Changes not staged for commit:
    atoi_git_status_files u;    // Untracked files:
} atoi_git_status;

void
atoi_git_init(const char *git_path, git_repository **repo);

void branch_install_prepare_git();

void pull_install_prepare_git(void);

void get_current_branch(char **cur_branch, git_repository *repo);
#endif /* defined(__atoi_git__) */
