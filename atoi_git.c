//
//  atoi_git.c
//  newInstall
//
//  Created by 张贺 on 15-5-31.
//  Copyright (c) 2015年 张贺. All rights reserved.
//


//static int
//atoi_get_git_status_cb(const char *path,
//                   unsigned int status_flags,
//                   void *payload)
//{
//    return status_flags;
//}

// 需要merge, checkout 时有文件改动未被提交
//if (git_status_foreach(repo, atoi_get_git_status_cb, NULL) != 0) {
//    
//    install_deb("Repository is not clean, need to commit...\n");
//    
//    atoi_git_commit_from_index(repo, "checkout commit");
//    
//}

#include "atoi_git.h"

extern struct install_options atoi_install_opt;

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
atoi_git_submodule_foreach_cb(git_submodule *sm, const char *name, void *payload);

//static int
//atoi_git_repository_clean(git_repository *repo);

static void
atoi_git_checkout_branch(git_repository *repo,
                         git_reference *new_branch_ref,
                         const char *target_branch_ref_name,
                         const char *install_name);

static void
atoi_git_commit_from_index(git_repository *repo, const char *commit_message);


void
atoi_git_init(const char *git_path, git_repository **repo)
{
    install_deb("Init git ...\n");
    git_err(git_libgit2_init());
    git_err(git_repository_open(repo, git_path));
}

static void
atoi_git_create_remote(git_repository *repo,
                       git_remote **remote,
                       const char *remote_name,
                       const char *url)
{
    install_deb("Create remote: %s ...\n", remote_name);
 
    int err = git_remote_lookup(remote, repo, remote_name);
    switch(err) {
        case 0:
            install_deb("Remote [%s] exist\n", remote_name);
            break;
        case GIT_ENOTFOUND:
            install_deb("Remote [%s] not exist. Creating ...\n", remote_name);
            git_err(git_remote_create(remote, repo, remote_name, url));
            break;
        default:
            git_err(err);
    }
}

static int
cred_acquire_cb(git_cred **out,
                const char * url,
                const char * username_from_url,
                unsigned int allowed_types,
                void* payload)
{
    char pub_key[256];
    char pri_key[256];
    
    //  ~/.ssh/id_rsa.pub
    snprintf(pub_key, 255, "%s/.ssh/id_rsa.pub", atoi_install_opt.home_dir);
    
    //  ~/.ssh/id_rsa
    snprintf(pri_key, 255, "%s/.ssh/id_rsa", atoi_install_opt.home_dir);

    git_err(git_cred_ssh_key_new(out,
                                 username_from_url,
                                 pub_key,
                                 pri_key,
                                 NULL));
    return 0;
}

static int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data)
{
    char a_str[GIT_OID_HEXSZ+1], b_str[GIT_OID_HEXSZ+1];
    (void)data;
    
    git_oid_fmt(b_str, b);
    b_str[GIT_OID_HEXSZ] = '\0';
    
    if (git_oid_iszero(a)) {
        printf("[new]     %.20s %s\n", b_str, refname);
    } else {
        git_oid_fmt(a_str, a);
        a_str[GIT_OID_HEXSZ] = '\0';
        printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
    }
    
    return 0;
}

static int
progress_cb(const char *str, int len, void *data)
{
    (void)data;
    printf("remote: %.*s", len, str);
    fflush(stdout);
    return 0;
}

static void
atoi_git_fetchcode(git_remote *remote)
{
    install_deb("Start fetch code...\n");
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
//    git_remote_callbacks rm_cb = GIT_REMOTE_CALLBACKS_INIT;

    const char *remote_name = git_remote_name(remote);
    const char *remote_url = git_remote_url(remote);
    install_deb("Remote name: [%s]\n", remote_name);
    install_deb("Remote URL:  [%s]\n", remote_url);
    
    // 设置 remote callback 回调函数属性
    fetch_opts.callbacks.update_tips = &update_cb;
    fetch_opts.callbacks.sideband_progress = &progress_cb;
    fetch_opts.callbacks.credentials = cred_acquire_cb;

    // 链接远程库
    install_mes("Connecting to remote [%s : %s] ...\n",
                remote_name,
                remote_url);
    git_err(git_remote_connect(remote, GIT_DIRECTION_FETCH, &fetch_opts.callbacks));

    // 更新代码
    install_mes("Fetching code        [%s : %s] ...\n",
                remote_name,
                remote_url);
    
    
    git_err(git_remote_download(remote, NULL, &fetch_opts));
    
    // 断开链接
    install_deb("Disconnect to remote [%s : %s]\n",
                remote_name,
                remote_url);
    git_remote_disconnect(remote);
    
    git_err(git_remote_update_tips(remote, &fetch_opts.callbacks, 1, fetch_opts.download_tags, NULL));
}

static int
atoi_git_submodule_foreach_cb(git_submodule *sm, const char *name, void *payload)
{
    int err = 0;
    // git submodule sync
    install_deb("sync submodule [%s]...\n", name);
    git_err(git_submodule_sync(sm));
    
    // git submodle update
    install_mes("Updating submodule: [%s]...\n", name);
    git_err(git_submodule_update(sm, 1, NULL));
    
//    git_submodule_free(sm);
    return err;
}

static void
atoi_git_checkout_branch(git_repository *repo,
                         git_reference *new_branch_ref,
                         const char *target_branch_ref_name,
                         const char *install_name)
{
    install_mes("Checkout to the new branch [%s => %s]\n",
                target_branch_ref_name,
                install_name);
    
    git_oid              target_branch_oid;
    git_object           *target_branch_obj = NULL;
    git_checkout_options checkout_opts      = GIT_CHECKOUT_OPTIONS_INIT;
    
    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    
    git_err(git_reference_name_to_id(&target_branch_oid, repo, target_branch_ref_name));
    install_deb("get oid from %s: %s\n",
                target_branch_ref_name,
                git_oid_tostr_s(&target_branch_oid));
    git_err(git_object_lookup(&target_branch_obj, repo, &target_branch_oid, GIT_OBJ_COMMIT));
    
    // 确定新创建的分支是否已经存在并且是当前分支
    if (git_branch_lookup(&new_branch_ref, repo, install_name, GIT_BRANCH_LOCAL) == 0) {
        if (git_branch_is_head(new_branch_ref) == 1) {
            install_deb("Current branch is [%s], "
                        "creat and checkout new branch [%s].\n",
                        install_name,
                        "master");

            git_reference *nnew_branch_ref = NULL;
            char *n_target_branch_ref_name = "refs/heads/master";
            atoi_git_checkout_branch(repo,
                                     nnew_branch_ref,
                                     n_target_branch_ref_name,
                                     n_target_branch_ref_name + (strlen(n_target_branch_ref_name) - strlen(strrchr(n_target_branch_ref_name, '/')) + 1));
        }
        // 删除分支
        
    }
    
    // create branch
    install_deb("creating new branch [%s]\n", install_name);
    git_err(git_branch_create(&new_branch_ref,
                              repo,
                              install_name,
                              (const git_commit*)target_branch_obj, 1));

    // checkout new branch
    install_mes("Checkout head to the new branch [%s]\n", install_name);
    git_err(git_repository_set_head(repo, git_reference_name(new_branch_ref)));
    
    install_deb("Checkout to the current head...\n");
    git_err(git_checkout_head(repo, &checkout_opts));
    
    // 更新 submodule
    install_mes("Update submodule ...\n");
    git_err(git_submodule_foreach(repo, atoi_git_submodule_foreach_cb, NULL));
    
    // 提交改动
    install_deb("Commit checkout files ...\n");
    atoi_git_commit_from_index(repo, atoi_install_opt.install_name);
    
    if (target_branch_obj != NULL)
        git_object_free(target_branch_obj);
}

static void
atoi_git_add_all(git_repository *repo)
{
    install_deb("Add all to index...\n");
    git_index *index;
//    git_strarray array = {0};
    
    git_repository_index(&index, repo);
    
    git_index_add_all(index, NULL, 0, NULL, NULL);
    git_index_write(index);
    git_index_free(index);
}

static void
atoi_git_commit_from_index(git_repository *repo, const char *commit_message)
{
    install_deb("git commit...\n");
    git_config    *conf      = NULL;
    git_signature *signature = NULL;
    git_index     *index     = NULL;
    git_tree      *tree      = NULL;
    git_commit    *commit    = NULL;
    git_oid tree_id, new_commit_id, commit_id;
    
//    const char *commit_user_name  = NULL;
//    const char *commit_user_email = NULL;
//    
//    install_deb("Open git config file...\n");
//    git_err(git_config_open_default(&conf));
//    
//    install_deb("Get user.name from git config file...\n");
//    git_err(git_config_get_string(&commit_user_name, conf, "user.name"));
//    
//    install_deb("Get user.email from git config file...\n");
//    git_err(git_config_get_string(&commit_user_email, conf, "user.email"));
    
    atoi_git_add_all(repo);
    
    git_err(git_signature_now(&signature, "gbyukg", commit_message));
    git_err(git_repository_index(&index, repo));
    git_err(git_index_write_tree_to(&tree_id, index, repo));
    git_err(git_tree_lookup(&tree, repo, &tree_id));
    git_err(git_reference_name_to_id(&commit_id, repo, "HEAD"));
    git_err(git_commit_lookup(&commit, repo, &commit_id));
    git_commit_create(&new_commit_id,
                      repo,
                      "HEAD",
                      signature,
                      signature,
                      NULL,
                      commit_message,
                      tree,
                      1,
                      (const git_commit**)&commit
                      );
    
    git_config_free(conf);
    git_index_free(index);
    git_signature_free(signature);
    git_commit_free(commit);
    git_tree_free(tree);
}

static int
atoi_git_status_cb(const char *path,
                   unsigned int status_flags,
                   void *payload)
{
    /* c: Changes to be commited
     a: Changes not staged for commit:
     u: Untracked files:
     */
    
    atoi_git_status *st = (atoi_git_status *) payload;
    
    if (status_flags == GIT_STATUS_IGNORED) {
        return 0;
    }
    
    if (status_flags & GIT_STATUS_INDEX_NEW) {
        st->c.path[st->c.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->c.path[st->c.count], "    new file:    ");
        strcat(st->c.path[st->c.count], (char *)path);
        st->c.count++;
    }
    if (status_flags & GIT_STATUS_INDEX_MODIFIED) {
        st->c.path[st->c.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->c.path[st->c.count], "    modified:    ");
        strcat(st->c.path[st->c.count], (char *)path);
        st->c.count++;
    }
    if (status_flags & GIT_STATUS_INDEX_DELETED) {
        st->c.path[st->c.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->c.path[st->c.count], "    deleted:     ");
        strcat(st->c.path[st->c.count], (char *)path);
        st->c.count++;
    }
    if (status_flags & GIT_STATUS_INDEX_RENAMED) {
        st->c.path[st->c.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->c.path[st->c.count], "    renamed:     ");
        strcat(st->c.path[st->c.count], (char *)path);
        st->c.count++;
    }
    if (status_flags & GIT_STATUS_INDEX_TYPECHANGE) {
        st->c.path[st->c.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->c.path[st->c.count], "    typechange:  ");
        strcat(st->c.path[st->c.count], (char *)path);
        st->c.count++;
    }
    if (status_flags & GIT_STATUS_WT_NEW) {
        st->u.path[st->u.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->u.path[st->u.count], "    ");
        strcat(st->u.path[st->u.count], (char *)path);
        st->u.count++;
    }
    if (status_flags & GIT_STATUS_WT_MODIFIED) {
        st->a.path[st->a.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->a.path[st->a.count], "    modified:    ");
        strcat(st->a.path[st->a.count], (char *)path);
        st->a.count++;
    }
    if (status_flags & GIT_STATUS_WT_DELETED) {
        st->a.path[st->a.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->a.path[st->a.count], "    deleted:     ");
        strcat(st->a.path[st->a.count], (char *)path);
        st->a.count++;
    }
    if (status_flags & GIT_STATUS_WT_TYPECHANGE) {
        st->a.path[st->a.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->a.path[st->a.count], "    typechange:  ");
        strcat(st->a.path[st->a.count], (char *)path);
        st->a.count++;
    }
    if (status_flags & GIT_STATUS_WT_RENAMED) {
        st->a.path[st->a.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->a.path[st->a.count], "    renamed:     ");
        strcat(st->a.path[st->a.count], (char *)path);
        st->a.count++;
    }
    if (status_flags & GIT_STATUS_WT_UNREADABLE) {
        st->a.path[st->a.count] = malloc((strlen(path) + 18) * sizeof(char));
        strcat(st->a.path[st->a.count], "    unreadable:  ");
        strcat(st->a.path[st->a.count], (char *)path);
        st->a.count++;
    }
    
    return 0;
}

static void
atoi_get_git_status(git_repository *repo)
{
    int i = 0;
    atoi_git_status st;
    git_status_foreach(repo, atoi_git_status_cb, &st);
    
    if (st.c.count > 0) {
        puts("Changes to be commited:");
        for (i = 0; i < st.c.count; i++) {
            printf("%s\n", st.c.path[i]);
            free(st.c.path[i]);
        }
    }
    if (st.a.count > 0) {
        puts("Changes not staged for commit:");
        for (i = 0; i < st.a.count; i++) {
            printf("%s\n", st.a.path[i]);
            free(st.a.path[i]);
        }
    }
    if (st.u.count > 0) {
        puts("Untracked files:");
        for (i = 0; i < st.u.count; i++) {
            printf("%s\n", st.u.path[i]);
            free(st.u.path[i]);
        }
    }
}

static void
atoi_git_merge(git_repository *repo, const char *refs_name)
{
    install_mes("Merage ref [%s] into current branch ...\n", refs_name);
    git_oid refs_oid;
    git_annotated_commit *refs_com = NULL;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE
    | GIT_CHECKOUT_ALLOW_CONFLICTS
    | GIT_CHECKOUT_REMOVE_UNTRACKED;
    
    git_err(git_reference_name_to_id(&refs_oid, repo, refs_name));
    git_err(git_annotated_commit_lookup(&refs_com, repo, &refs_oid));
    git_err(git_merge(repo,
                      (const git_annotated_commit **)&refs_com,
                      1,
                      &merge_opts,
                      &checkout_opts)
            );
    
    // 获取当前状态信息
    atoi_get_git_status(repo);
    
    // 清理操作
    git_err(git_repository_state_cleanup(repo));
}

void branch_install_prepare_git(void)
{
    install_deb("Running hook [before_fetch_code]");
    if (install_hook("before_fetch_code", NULL) != 0)
        extErr("Run hook [before_fetch_code] wrong!");
    
    git_repository *repo = NULL;
    git_remote *remote = NULL;
    static char update_remote[256];
    char refs_name[256] = "";
    char url[256];
    
    _atoi_branch_info_ *bc = atoi_install_opt.branch_info;
    
    atoi_git_init(atoi_install_opt.git_path, &repo);
    
    while (bc != NULL) {
        snprintf(refs_name,
                 sizeof(refs_name),
                 "refs/remotes/%s/%s",
                 bc->remote,
                 bc->branch_name);
        snprintf(url, sizeof(url), "git@github.com:%s/Mango.git", bc->remote);
        
        atoi_git_create_remote(repo,
                               &remote,
                               bc->remote,
                               url);
        
        // 是否需要更新远程库, 同一用户下的多个分支只需更新一次该用户的远程库
        if (strstr(update_remote, bc->remote) == NULL) {
            atoi_git_fetchcode(remote);
            strcat(update_remote, bc->remote);
        }
        
        // 判断为第一个安装分支, 将该分支作为基准分支创建新branch
        if (atoi_install_opt.branch_info->index == 0) {
            git_reference *new_branch_ref = NULL;
            atoi_git_checkout_branch(repo, new_branch_ref, refs_name, atoi_install_opt.install_name);
        } else {
            // merge 其他分支
            atoi_git_merge(repo, refs_name);
        }

        if (remote != NULL)
            git_remote_free(remote);
        bc = bc->next;
    }
    
    atoi_git_commit_from_index(repo, "branch install");
    
    if (repo != NULL)
        git_repository_free(repo);
    git_err(git_libgit2_shutdown());
    
    install_deb("Running hook [after_fetch_code]");
    if (install_hook("after_fetch_code", NULL) != 0)
        extErr("Run hook [before_fetch_code] wrong!");
}

void pull_install_prepare_git(void)
{
    install_deb("Into [pull_install_prepare_git] methord...\n");
    // base refs
    char base_refs_name[256] = "";
    snprintf(base_refs_name,
             sizeof(base_refs_name),
             "refs/remotes/%s/%s",
             atoi_install_opt.pull_info->base_user_login,
             atoi_install_opt.pull_info->base_ref);
    
    // head refs
    char head_refs_name[256] = "";
    snprintf(head_refs_name,
             sizeof(head_refs_name),
             "refs/remotes/%s/%s",
             atoi_install_opt.pull_info->head_user_login,
             atoi_install_opt.pull_info->head_ref);
    install_deb("base_refs_name [%s]\n", base_refs_name);
    install_deb("head_refs_name [%s]\n", head_refs_name);
    
    git_repository *repo = NULL;
    
    // 初始化 git 仓库
    atoi_git_init(atoi_install_opt.git_path, &repo);
    
    git_remote *base_remote = NULL;
    git_remote *head_remote = NULL;
    
    atoi_git_create_remote(repo,
                           &base_remote,
                           atoi_install_opt.pull_info->base_user_login,
                           atoi_install_opt.pull_info->base_repo_ssh_url);
    atoi_git_create_remote(repo,
                           &head_remote,
                           atoi_install_opt.pull_info->head_user_login,
                           atoi_install_opt.pull_info->head_repo_ssh_url);
    // fetch 代码
    atoi_git_fetchcode(base_remote);
    atoi_git_fetchcode(head_remote);
    
    // create new branch from head
    git_reference *new_branch_ref = NULL;
    atoi_git_checkout_branch(repo, new_branch_ref, base_refs_name, atoi_install_opt.install_name);
    
    // merge base branch
    atoi_git_merge(repo, head_refs_name);
    
//    if (new_branch_ref != NULL)
//        git_reference_free(new_branch_ref);

    // 释放资源
//    if (base_refs != NULL)
//        git_reference_free(base_refs);
//    if (head_refs != NULL)
//        git_reference_free(head_refs);
//    git_reference_free(new_branch_ref);
    if (base_remote != NULL)
        git_remote_free(base_remote);
    if (head_remote != NULL)
        git_remote_free(head_remote);
    if (repo != NULL)
        git_repository_free(repo);
    git_err(git_libgit2_shutdown());
}

void get_current_branch(char **cur_branch, git_repository *repo)
{
    git_reference *cur_refs = NULL;
    
    git_err(git_repository_head(&cur_refs, repo));
    git_err(git_branch_name((const char**)cur_branch, cur_refs));
}