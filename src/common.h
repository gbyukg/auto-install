//
//  common.h
//  newInstall
//
//  Created by 张贺 on 15-5-27.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#ifndef __atoi_install__common__
#define __atoi_install__common__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "atoi_curl.h"

#ifdef SERVERINSTALL
#define VERSION "server"
#else
#define VERSION "local"
#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


#define BUF_SIZE BUFSIZ

#define CURTIME(fd) {\
char cur_time[64] = {'\0'}; \
time_t t = time(NULL); \
strftime(cur_time, 63, "%c", localtime(&t)); \
fprintf(fd, KBLU"%s"KNRM, cur_time); \
fflush(NULL); \
}

#define DEB_CURTIME(fd) {\
char cur_time[64] = {'\0'}; \
time_t t = time(NULL); \
strftime(cur_time, 63, "%c", localtime(&t)); \
fprintf(fd, "%s", cur_time); \
fflush(NULL); \
}

#define SYSCALL(func) ({\
int rtCode = -1; /* return code */ \
if ((rtCode = func) == -1) {\
if(log_file != NULL) {\
DEB_CURTIME(stderr);\
perror(": System call [" KMAG #func KNRM "] wrong"KRED);\
fprintf(stderr,\
"** FILE: %s\n"\
"** FUNC: %s\n"\
"** LINE: %d\n",\
__FILE__,\
__FUNCTION__,\
__LINE__);\
}\
CURTIME(stderr);\
perror(": System call [" KMAG #func KNRM "] wrong"KRED);\
fprintf(stderr,\
KNRM"** FILE: "KCYN"%s"KNRM"\n"\
"** FUNC: "KCYN"%s"KNRM"\n"\
"** LINE: "KCYN"%d"KNRM"\n",\
__FILE__,\
__FUNCTION__,\
__LINE__);\
exit(EXIT_FAILURE);\
}\
rtCode;\
})

#define atoi_malloc(len) ({\
void *__atoi_malloc = malloc(len);\
if (__atoi_malloc == NULL)\
extErr("Malloc wrong!");\
__atoi_malloc;\
})

#define CUS_SYSCALL(func) ({\
int rtCode = -1; /* return code */ \
if ((rtCode = func) == -1) {\
if(log_file != NULL) {\
DEB_CURTIME(log_file); \
fprintf(stderr,\
": Custom call [" #func "] wrong: %s; ", atoi_err_mes.err_mes);\
fprintf(stderr,\
"** FILE: %s; "\
"** FUNC: %s; "\
"** LINE: %d\n",\
__FILE__,\
__FUNCTION__,\
__LINE__);\
}\
CURTIME(stderr);\
fprintf(stderr,\
": Custom call [" KMAG #func KNRM "] wrong: %s\n"KRED, atoi_err_mes.err_mes);\
fprintf(stderr,\
KNRM"** FILE: "KCYN"%s"KNRM"\n"\
"** FUNC: "KCYN"%s"KNRM"\n"\
"** LINE: "KCYN"%d"KNRM"\n",\
__FILE__,\
__FUNCTION__,\
__LINE__);\
exit(EXIT_FAILURE);\
}\
rtCode;\
})

#define extErr(format, ...) { \
if(log_file != NULL) {\
DEB_CURTIME(log_file); \
fprintf(log_file, \
": "format "; [FILE]: %s; "\
"[FUNC]: %s; "\
"[LINE]: %d\n",\
##__VA_ARGS__, \
__FILE__,\
__FUNCTION__,\
__LINE__);\
}\
CURTIME(stderr); \
fprintf(stderr, \
": "format KNRM"\n** FILE: "KCYN"%s"KNRM"\n"\
"** FUNC: "KCYN"%s"KNRM"\n"\
"** LINE: "KCYN"%d"KNRM"\n",\
##__VA_ARGS__, \
__FILE__,\
__FUNCTION__,\
__LINE__);\
exit(EXIT_FAILURE); \
}

#define install_deb(format, ...) ({\
if(log_file != NULL) {\
DEB_CURTIME(log_file); \
fprintf(log_file, \
" Debug Message: " format, ##__VA_ARGS__);\
}\
if (atoi_install_opt.debug) {\
CURTIME(stdout); \
fprintf(stdout, \
KCYN " Debug Message: "KNRM format, ##__VA_ARGS__);\
}\
})

#define install_mes(format, ...) {\
if(log_file != NULL) {\
DEB_CURTIME(log_file); \
fprintf(log_file, \
" Info: " format, ##__VA_ARGS__);\
CURTIME(stdout); \
}\
fprintf(stdout, \
KGRN " Info: "KNRM format, ##__VA_ARGS__);\
}

#define PULL_INSTALL    1
#define BRANCH_INSTALL  2
#define GIT_INSTALL     4
#define WEB_DIR_INSTALL 8
#define PACKAGE_INSTALL 16
#define CR_PR           32

FILE *log_file;

/*
 * pull request 信息
 */
typedef struct _atoi_pull_info
{
    char *pull_number;      // pull request number
    char *diff_url;        // diff_url
    char *patch_url;       // patch_url
    char *state;           // open/close
    char *title;           // pull request title
    char *body;            // pull request message
    char *user_login;      // Github login name
    char *head_ref;        // ibmd_60159
    char *head_user_login; // gbyukg
    char *head_repo_name;  // Mango
    char *head_repo_ssh_url;// git@github.com:gbyukg/Mango.git
    char *base_ref;        // ibm_r22
    char *base_user_login; // sugareps
    char *base_repo_name;  // Mango
    char *base_repo_ssh_url;// git@github.com:sugareps/Mango.git
} _atoi_pull_info_;

typedef struct _atoi_branch_info
{
    int index;
    char remote[64];
    char branch_name[64];
    struct _atoi_branch_info *next;
} _atoi_branch_info_;

typedef struct _atoi_cr_pr
{
    char *bash_ref;
    char *head_ref;
} _atoi_cr_pr_;

/*
 * 安装选项
 */
struct install_options {
    int install_method; // 安装方式: pull request 和 branch
    int debug;        // 输出安装详细过程
    int dataloader;     // 导入数据
    int cp_dataloader;
    int avl;            // 导入 ACL
    int unittest;       // 执行PHP unit test
    int build_code_or_not;     //
    int run_after_install;
    int init_db_or_not;        //
    int update_composer;        // 是否更新 composer, 默认更新
    int keep_live;      // 安装实例保留的时间, 默认3天(server 使用)
    char *cur_dir;      // 当前程序执行路径
    char *dataloader_dir;
    const char *install_hook_script;
    char *install_name; // install name
    const char *init_db_script;
    const char *token;
    
    const char *git_path;     // git 路径
    const char *web_path;     // web 路径
    const char *build_path;   // build 路径
    const char *def_base_user;// sugareps
    const char *def_head_user;// gbyukg
    const char *home_dir;     // home
    
    const char *sc_license;
    const char *web_host;     // http://localhost
    const char *sc_admin;
    const char *sc_admin_pwd;
    
    const char *dbname;       // 安装时使用的数据库名, 最大长度31个字节
    const char *db_port;
    const char *db_host;
    const char *db_admin;
    const char *db_admin_pwd;
    
    const char *fts_type;
    const char *fts_host;
    const char *fts_port;
    
    const char *tmp_path;
    
    const char git_token[64]; // github token
    union {
        _atoi_pull_info_   *atoi_pull_info;
        _atoi_branch_info_ *atoi_branch_info;
        _atoi_cr_pr_       *atoi_cr_pr;
        char *package_info;
    } __install_paramater;
#define pull_info    __install_paramater.atoi_pull_info
#define branch_info  __install_paramater.atoi_branch_info
#define package_info __install_paramater.package_info
#define cr_pr        __install_paramater.atoi_cr_pr
};

/*
 * 自定义错误信息
 */
typedef struct _atoi_err_mes_ {
    int code;          // 错误代码
    char err_mes[128]; // 错误信息
} _atoi_err_mes;

extern int errno;

int
install_hook(const char *com, const char *param);

/*
 * 执行shell命令
 */
int
shell_call(int n, const char *command, ...);

int analyse_branch(const char *get_branch_info);

#endif /* defined(__atoi_install__common__) */
