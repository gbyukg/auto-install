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

static void
atoi_git_create_remote(git_repository *repo,
                       git_remote **remote,
                       const char *remote_name,
                       const char *url);

static int
cred_acquire_cb(git_cred **out,
                const char * url,
                const char * username_from_url,
                unsigned int allowed_types,
                void* payload);

static int
progress_cb(const char *str, int len, void *data);

/**
 *  Fetch code from remote
 *
 *  @param remote git_remote
 */
static void
atoi_git_fetchcode(git_remote *remote);

static int
update_cb(const char *refname,
                     const git_oid *a,
                     const git_oid *b,
                     void *data);

static int
atpi_git_submodule_foreach_cb(git_submodule *sm, const char *name, void *payload);

static int
atoi_git_repository_clean(git_repository *repo);

static void
atoi_git_checkout_branch(git_repository *repo,
                         git_reference *new_branch_ref,
                         const char *target_branch_ref_name,
                         const char *install_name);

void branch_install_prepare_git();

static void
atoi_git_commit_from_index(git_repository *repo, const char *commit_message);

void pull_install_prepare_git(void);

#endif /* defined(__atoi_git__) */
