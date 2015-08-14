//
//  atoi_install.c
//  newInstall
//
//  Created by 张贺 on 15-5-28.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#include <dirent.h>
#include "atoi_install.h"

extern int errno;
extern char curl_err_buf[];
extern struct install_options atoi_install_opt;

static void
after_install(void);

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

static void
pre_install(void);

//static void
//package_upgrade(const char *package_path);

static long curl_install_step(const char *post);

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

char *get_cur_branch_name()
{
    git_repository *repo = NULL;
    git_reference *ref = NULL;
    char *branch_name = NULL;
    
    atoi_git_init(atoi_install_opt.git_path, &repo);
    
    git_repository_head(&ref, repo);
    git_branch_name((const char**)&branch_name, ref);
    
    git_reference_free(ref);
    git_repository_free(repo);
    return branch_name;
}

YAJL_API yajl_val atoi_yajl_tree_get(yajl_val *node, const char **name, int type)
{
    int i = 0;
    yajl_val jajlVal = yajl_tree_get(*node, name, type);
    if (jajlVal == NULL) {
        fprintf(stderr, "No [");
        while (name[i] != NULL) {
            fprintf(stderr, "%s ", name[i]);
            i++;
        }
        extErr("] node in response from json. Pull request number is incorrect.");
    }
    return jajlVal;
}

void pull_install()
{
    install_mes("Install from pull request [%s]\n", atoi_install_opt.pull_info->pull_number);

    errno = 0;
    struct MemoryStruct chunk;
    char errbuf[1024];
    yajl_val node = NULL;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;            /* no data at this point */
    
    char pull_url[64] = "";
    snprintf(pull_url,
             63,
             "https://api.github.com/repos/sugareps/Mango/pulls/%s",
             atoi_install_opt.pull_info->pull_number);
    
    install_deb("Pull request URL: [%s]\n", pull_url);
    
    CURLcode returnCode = CURLE_OK;
    struct curl_slist *headers = NULL;
    char token[128] = "Authorization: token %s";
    if (atoi_install_opt.token == NULL) {
        extErr("token is null.\n");
    }
    snprintf(token,
             sizeof(char) * 62, // 注意长度, 直接用 sizeof(token) 导致多余空白字符被传递过去, 认证会失败
             token,
             atoi_install_opt.token);
    
    headers = curl_slist_append(headers, token);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    CURL *curl = atoi_get_curl(pull_url, headers);
    if (!curl) {
        extErr("CURL 初始化失败");
    }

    // 设置连接超时时间
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 90L);
    // 设置解析时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    
    /*
     * 如果使用了 CURLOPT_WRITEFUNCTION 属性, pointer 则会被传递给
     * write_callback 的第四个参数. 默认情况下 CURLOPT_WRITEFUNCTION 会把接收到的数据打印到标准输出,
     * 如果 CURLOPT_WRITEDATA 传递的是一个 `FILE *'` 类型, 则获取到的数据会被写入到该文件流中
     *
     * 如果没有使用 CURLOPT_WRITEFUNCTION 属性, pointer 必须指定为一个 `FILE *'` 类型
     * 将获取到的数据写入到 `FILE *'` 中.
     */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
//    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, pull_url);
    
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 90L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    
    install_mes("Getting pull request [%s] info ...\n", atoi_install_opt.pull_info->pull_number);
    if ((returnCode = curl_easy_perform(curl)) != CURLE_OK) {
        extErr("curl wrong: [%s]!", curl_err_buf);
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        extErr("Pull request number [%s] is incorrect!", atoi_install_opt.pull_info->pull_number);
    }
    
    install_deb("开始解析json\n");
    
    // man info
    const char * diff_url[]          = { "diff_url", (const char *) 0 };
    const char * patch_url[]         = { "patch_url", (const char *) 0 };
    const char * state[]             = { "state", (const char *) 0 };
    const char * title[]             = { "title", (const char *) 0 };
    const char * body[]              = { "body", (const char *) 0 };
    const char * user_login[]        = { "user", "login", (const char *) 0 };
    // header info
    const char * head_ref[]          = { "head", "ref", (const char *) 0 };
    const char * head_user_login[]   = { "head", "user", "login", (const char *) 0 };
    const char * head_repo_name[]    = { "head", "repo", "name", (const char *) 0 };
    const char * head_repo_ssh_url[] = { "head", "repo", "ssh_url", (const char *) 0 };
    // base info
    const char * base_ref[]          = { "base", "ref", (const char *) 0 };
    const char * base_user_login[]   = { "base", "user", "login", (const char *) 0 };
    const char * base_repo_name[]    = { "base", "repo", "name", (const char *) 0 };
    const char * base_repo_ssh_url[] = { "base", "repo", "ssh_url", (const char *) 0 };
    
    if ((node = yajl_tree_parse((const char *) chunk.memory, errbuf, sizeof(errbuf))) == NULL) {
        extErr("yajl_tree_parse wrong");
    }
    
    yajl_val v_diff_url          = atoi_yajl_tree_get(&node, diff_url, yajl_t_string);
    yajl_val v_patch_url         = atoi_yajl_tree_get(&node, patch_url, yajl_t_string);
    yajl_val v_state             = atoi_yajl_tree_get(&node, state, yajl_t_string);
    yajl_val v_title             = atoi_yajl_tree_get(&node, title, yajl_t_string);
    yajl_val v_body              = atoi_yajl_tree_get(&node, body, yajl_t_string);
    yajl_val v_user_login        = atoi_yajl_tree_get(&node, user_login, yajl_t_string);
    
    yajl_val v_head_ref          = atoi_yajl_tree_get(&node, head_ref, yajl_t_string);
    yajl_val v_head_user_login   = atoi_yajl_tree_get(&node, head_user_login, yajl_t_string);
    yajl_val v_head_repo_name    = atoi_yajl_tree_get(&node, head_repo_name, yajl_t_string);
    yajl_val v_head_repo_ssh_url = atoi_yajl_tree_get(&node, head_repo_ssh_url, yajl_t_string);
    
    yajl_val v_base_ref          = atoi_yajl_tree_get(&node, base_ref, yajl_t_string);
    yajl_val v_base_user_login   = atoi_yajl_tree_get(&node, base_user_login, yajl_t_string);
    yajl_val v_base_repo_name    = atoi_yajl_tree_get(&node, base_repo_name, yajl_t_string);
    yajl_val v_base_repo_ssh_url = atoi_yajl_tree_get(&node, base_repo_ssh_url, yajl_t_string);
    
    atoi_install_opt.pull_info->diff_url          = strndup(YAJL_GET_STRING(v_diff_url), 64);
    atoi_install_opt.pull_info->patch_url         = strndup(YAJL_GET_STRING(v_patch_url), 64);
    atoi_install_opt.pull_info->state             = strndup(YAJL_GET_STRING(v_state), 64);
    atoi_install_opt.pull_info->title             = strndup(YAJL_GET_STRING(v_title), 512);
    atoi_install_opt.pull_info->body              = strndup(YAJL_GET_STRING(v_body), 512);
    atoi_install_opt.pull_info->user_login        = strndup(YAJL_GET_STRING(v_user_login), 64);
    
    atoi_install_opt.pull_info->head_ref          = strndup(YAJL_GET_STRING(v_head_ref), 64);
    atoi_install_opt.pull_info->head_user_login   = strndup(YAJL_GET_STRING(v_head_user_login), 64);
    atoi_install_opt.pull_info->head_repo_name    = strndup(YAJL_GET_STRING(v_head_repo_name), 64);
    atoi_install_opt.pull_info->head_repo_ssh_url = strndup(YAJL_GET_STRING(v_head_repo_ssh_url), 64);
    
    atoi_install_opt.pull_info->base_ref          = strndup(YAJL_GET_STRING(v_base_ref), 64);
    atoi_install_opt.pull_info->base_user_login   = strndup(YAJL_GET_STRING(v_base_user_login), 64);
    atoi_install_opt.pull_info->base_repo_name    = strndup(YAJL_GET_STRING(v_base_repo_name), 64);
    atoi_install_opt.pull_info->base_repo_ssh_url = strndup(YAJL_GET_STRING(v_base_repo_ssh_url), 64);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    yajl_tree_free(node);
    
    install_deb("diff_url:          %s\n", atoi_install_opt.pull_info->diff_url);
    install_deb("patch_url:         %s\n", atoi_install_opt.pull_info->patch_url);
    install_deb("state:             %s\n", atoi_install_opt.pull_info->state);
    install_deb("title:             %s\n", atoi_install_opt.pull_info->title);
    install_deb("body:              %s\n", atoi_install_opt.pull_info->body);
    install_deb("user_login:        %s\n", atoi_install_opt.pull_info->user_login);
    install_deb("head_ref:          %s\n", atoi_install_opt.pull_info->head_ref);
    install_deb("head_user_login:   %s\n", atoi_install_opt.pull_info->head_user_login);
    install_deb("head_repo_name:    %s\n", atoi_install_opt.pull_info->head_repo_name);
    install_deb("head_repo_ssh_url: %s\n", atoi_install_opt.pull_info->head_repo_ssh_url);
    install_deb("base_ref:          %s\n", atoi_install_opt.pull_info->base_ref);
    install_deb("base_user_login:   %s\n", atoi_install_opt.pull_info->base_user_login);
    install_deb("base_repo_name:    %s\n", atoi_install_opt.pull_info->base_repo_name);
    install_deb("base_repo_ssh_url: %s\n", atoi_install_opt.pull_info->base_repo_ssh_url);
    
    char install_name[64];
    snprintf(install_name, 63, "pull_install_%s", atoi_install_opt.pull_info->pull_number);
    if (atoi_install_opt.install_name == NULL) {
        atoi_install_opt.install_name = strndup(install_name, strlen(install_name));
    }
    install_mes("Install name [%s]\n",
                atoi_install_opt.install_name);
    
    pull_install_prepare_git();
    start_install();
}

void branch_install()
{
    install_mes("Install from branch...\n");
    branch_install_prepare_git();
    start_install();
}

void package_install(const char *get_package_info)
{
    // 禁止 build 代码
    atoi_install_opt.build_code_or_not = 0;
    // 不跑 after_install hook
    atoi_install_opt.run_after_install = 0;
    // 禁止更新composer
    // atoi_install_opt.update_composer = 0;
    
    install_mes("Install from package\n");
    const char *install_name = "package_install";
    if (atoi_install_opt.install_name == NULL)
        atoi_install_opt.install_name = strdup(install_name);
    
    int i = 0;
    char *install_package = atoi_malloc(strlen(get_package_info) + sizeof(char));
    time_t now;
    time(&now); // 缓存目录名
    
    strcpy(install_package, get_package_info);
    char *upgrade_package = strtok(install_package, " ");

    while (upgrade_package != NULL) {
        if (i == 0) {
            // 安装基础包
            if (strstr(upgrade_package, "://") != NULL) {
                install_mes("Install and download base package [%s]\n", upgrade_package);
                if (strstr(upgrade_package, ".zip") != NULL) {
                    const char *package_name = strrchr(upgrade_package, '/');
                    char param[256];
                    char dataloader_path[256];
                    // url
                    // 包名
                    // 缓存目录名
                    snprintf(param, 255, "url \"%s\" \"%s\" \"%ld\"", upgrade_package, package_name+1, now);
                    if (install_hook("package_install", param) != 0)
                        extErr("Can not download package from [%s]", upgrade_package);
                    
                    // 使用升级包中的 dataloader 文件
                    atoi_install_opt.cp_dataloader = 1;
                    snprintf(dataloader_path, 255, "%s%ld/ibm/dataloaders", atoi_install_opt.tmp_path, now);
                    atoi_install_opt.dataloader_dir = strdup(dataloader_path);
                    install_deb("Data loader folder [%s]\n", atoi_install_opt.dataloader_dir);
                    start_install();
                } else {
                    extErr("下载文件必须为 zip 文件: [%s]", upgrade_package);
                }
            } else {
                install_mes("Install from local package [%s]\n", upgrade_package);
                if (strstr(upgrade_package, ".zip") != NULL) {
                    // zip 文件
                    char param[256];
                    snprintf(param, 255, "lczip \"\" \"%s\" \"%ld\"", upgrade_package, now);
                    if (install_hook("package_install", param) != 0)
                        extErr("解压缩文件 [%s] 失败!", upgrade_package);
                    start_install();
                } else {
                    extErr("无效的压缩包文件!");
                }
            }
        } else {
            // 安装升级包
            char param[256];
            snprintf(param, 255, "upgrade \"NULL\" \"%s\" \"%ld\"", upgrade_package, now);
            install_mes("\nUpdate package [%s]\n", upgrade_package);
            if (install_hook("package_install", param) != 0)
                extErr("解压缩文件 [%s] 失败!", upgrade_package);
        }
        i++;
        upgrade_package = strtok(NULL, " ");
        
        // run dataloader
        if (upgrade_package == NULL) {
            if (install_hook("after_package_install", NULL) != 0)
                extErr("执行hook [after_package_install] 失败!");
        }
    }
    
    // dataloader
    after_install();
    
    free(install_package);
}

static void
pre_install()
{
    pid_t childPid, dl_pid;
    int status = 0, child_exit = 0;
    sigset_t block_sig, origin_sig;
    
    if (strlen(atoi_install_opt.install_name) == 0) {
        extErr("Install name is empty!");
    }
    if (strlen(atoi_install_opt.dbname) == 0) {
        extErr("DB name is empty!");
    }
    
    install_mes("Init db and build code...\n");
    switch (childPid = fork()) {
        case -1:
            extErr("Fork error...");
            break;
        case 0:
            
            // 禁止中断数据库的创建
            sigemptyset(&block_sig);
            sigemptyset(&origin_sig);
            sigaddset(&block_sig, SIGINT);
            sigprocmask(SIG_SETMASK, &block_sig, &origin_sig);
            
            install_deb("init_db_or_not: [%d]\n", atoi_install_opt.init_db_or_not);
            if (atoi_install_opt.init_db_or_not) {
                install_mes("Initializing database [%s]\n", atoi_install_opt.dbname);
                
                if (install_hook("before_init_db", NULL) != 0) {
                    child_exit = 300;
                    goto ERR;
                }
                
                if (shell_call(1, atoi_install_opt.init_db_script, atoi_install_opt.dbname) != 0) {
                    child_exit = 301;
                    goto ERR;
                }
                
                if (install_hook("after_init_db", NULL) != 0) {
                    child_exit = 302;
                    goto ERR;
                }
            }
        ERR:
            sigprocmask(SIG_SETMASK, &origin_sig, NULL);
            _exit(child_exit);
        default:
            switch (dl_pid = fork()) {
                case -1:
                    extErr("Fork error...");
                    break;
                case 0:
//                    if (atoi_install_opt.dataloader && atoi_install_opt.cp_dataloader) {
//                        install_deb("Copy dataloader\n");
//                        char dl_target[256];
//                        char cp_command[256];
//                        snprintf(dl_target, 255, "%s-dataloaders", atoi_install_opt.install_name);
//                        atoi_install_opt.dataloader_dir = strdup(dl_target);
//                                                snprintf(cp_command, 255, "cp -R %s/ibm/dataloaders %s%s",
//                                                         atoi_install_opt.tmp_path,
//                                                         atoi_install_opt.git_path, atoi_install_opt.dataloader_dir);
//                        if (shell_call(0, cp_command) != 0)
//                            extErr("Copying dataloader wrong!");
//                    }
                    exit(EXIT_SUCCESS);
                    break;
                default:
                    install_deb("build_code_or_not: [%d]\n", atoi_install_opt.build_code_or_not);
                    if (atoi_install_opt.install_method & 7 && atoi_install_opt.build_code_or_not) {
                        // build code from git repository
                        install_mes("Building code into [%s]...\n", atoi_install_opt.build_path);
                        if (install_hook("build_code", NULL) != 0)
                            extErr("Run hook [build_code] wrong!");
                    }
                    break;
            }
            break;
    }
    
    while (waitpid(-1, &status, 0) > 0) {
        if (WIFEXITED(status)) {
            // 子进程正常结束
            if (WEXITSTATUS(status) != 0)
                extErr("Init db [%s] wrong! Return value: [%d]", atoi_install_opt.dbname, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            // 子进程因信号被杀掉
            extErr("Child[%d] is killed by signal[%d]", childPid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            // 子进程因信号而停止
            extErr("Child[%d] is stoped by signal[%d]", childPid, WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            extErr("Child[%d] catch signal[%d]", childPid, WSTOPSIG(status));
        }
    }
}

static size_t
no_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

static long curl_install_step(const char *post)
{
    struct curl_slist *headers = NULL;
    char install_url[512];
    char host[512];
    char ref[512];
    char cookie_file[512];
    char *scp_web_host = strdup(atoi_install_opt.web_host);
    int debug = atoi_install_opt.debug;
    
    snprintf(cookie_file, 511, "%s%d-atoi.cookie", atoi_install_opt.tmp_path, getpid());
    install_deb("Cookie file: [%s]\n", cookie_file);
    
    snprintf(install_url, 511, "%s/%s/install.php", atoi_install_opt.web_host, atoi_install_opt.install_name);
    snprintf(host, 511, "Host: %s", scp_web_host + 7);
    snprintf(ref, 511, "Referer: %s", install_url);
    install_deb("Install url: %s\n", install_url);
    install_deb("host: %s\n", host);
    install_deb("ref: %s\n", ref);
    
    if (post != NULL) install_deb("%s\n", post);
    atoi_install_opt.debug = 0;
    
//    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
//    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, sdch");
//    headers = curl_slist_append(headers, "Connection: keep-alive");
//    headers = curl_slist_append(headers, "Accept-Language: en,zh-CN;q=0.8,zh;q=0.6");
//    headers = curl_slist_append(headers, "Connection: keep-alive");
//    headers = curl_slist_append(headers, "Content-Type: text/html");
//    headers = curl_slist_append(headers, host);
//    headers = curl_slist_append(headers, ref);
    CURL *curl = atoi_get_curl(install_url, headers);
    atoi_install_opt.debug = debug;
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, no_write_callback);
    if (post != NULL)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    return http_code;
}

void
start_install()
{
    install_mes("Starting install [%s]...\n", atoi_install_opt.install_name);
    
    pre_install();
    
    char dir_path[256];
    snprintf(dir_path, 255, "%s%s", atoi_install_opt.web_path, atoi_install_opt.install_name);
    DIR* dir = opendir(dir_path);
    
    if (dir)
    {
        /* Directory exists. */
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        extErr("安装文件 [%s] 不存在", dir_path);
        /* Directory does not exist. */
    }
    
    FILE *sfd = NULL;
    char param[BUF_SIZE];
    char *lineptr = (char *)malloc(BUF_SIZE);
    size_t line_len = BUF_SIZE;
    char fileName[512];
    snprintf(fileName, 511, "%s%d-atoi-XXXXXX", atoi_install_opt.tmp_path, getpid());

    install_mes("Start installing...\n");
    sprintf(param, "\"%d\"", atoi_install_opt.update_composer);
    if (install_hook("before_install", param) != 0)
        extErr("Run hook [before_install] wrong!");
    
    memset(param, 0, BUF_SIZE);
    int temFd = SYSCALL(mkstemp(fileName));
    if ((sfd = fdopen(temFd, "r+")) == NULL)
        extErr("fdopen wrong!");

    sprintf(param, "\"%s\" \"%s/%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
            fileName,
            atoi_install_opt.web_host,
            atoi_install_opt.install_name,
            atoi_install_opt.sc_license,
            atoi_install_opt.fts_type,
            atoi_install_opt.fts_host,
            atoi_install_opt.fts_port,
            atoi_install_opt.dbname,
            atoi_install_opt.db_port,
            atoi_install_opt.db_host,
            atoi_install_opt.db_admin,
            atoi_install_opt.db_admin_pwd,
            atoi_install_opt.sc_admin,
            atoi_install_opt.sc_admin_pwd);

    if (install_hook("install", param) != 0)
        extErr("Run hook [install] wrong!");

    curl_install_step(NULL);
    int cur_step = 0;
    long response_code = 0;
    
    while (getline(&lineptr, &line_len, sfd) > 0) {
        install_mes("Current step [%d]\n", cur_step);
        lineptr[strlen(lineptr) - 1] = '\0';
        
        response_code = curl_install_step(lineptr);
        
        if (response_code != 200) {
            install_mes("Params: %s\n", lineptr);
            extErr("Wrong response code [%ld]", response_code);
        }
        install_mes("Response code: %ld\n", response_code);
        cur_step++;
    }
    curl_install_step(NULL);
    
    free(lineptr);
    fclose(sfd);
    unlink(fileName);
    
    if (atoi_install_opt.run_after_install == 1)
        after_install();
}

static void
after_install()
{
    // dataloader
    if (atoi_install_opt.dataloader) {
        
        install_mes("data loader...\n");
        if (install_hook("dataloader", NULL) != 0)
            extErr("Run hook [build_code] wrong!");
    }
    
    // AVL
    if (atoi_install_opt.avl) {
        install_mes("Importing AVL ...\n");
        if (install_hook("load_avl", NULL) != 0)
            extErr("Run hook [build_code] wrong!");
    }
    
    // PHP unit test
    if (atoi_install_opt.unittest) {
        install_mes("Running PHP Unittest ...\n");
        if (install_hook("run_ut", NULL) != 0)
            extErr("Run hook [build_code] wrong!");
    }
    
    // after install logichook
    install_mes("After install ...\n");
    if (install_hook("after_install", NULL) != 0)
        extErr("Run hook [build_code] wrong!");
}
