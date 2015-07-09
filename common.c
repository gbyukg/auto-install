//
//  common.c
//  newInstall
//
//  Created by 张贺 on 15-5-27.
//  Copyright (c) 2015年 张贺. All rights reserved.
//
#include "common.h"

extern struct install_options atoi_install_opt;

int install_hook(const char *func, const char *param)
{
    char hook[1024] = "";
    
    /*
     * arg1: fun_name
     * arg2: install name
     * arg3: git dir
     * arg4: web dir
     * arg5: build dir
     */
    snprintf(hook, 1023, "\"%s\"  \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \%s",
             atoi_install_opt.install_hook_script,
             atoi_install_opt.debug == 1 ? "debug" : "no-debug",
             func,
             atoi_install_opt.install_name,
             atoi_install_opt.git_path,
             atoi_install_opt.web_path,
             atoi_install_opt.build_path,
             param == NULL ? "" : param);
    return shell_call(0, hook);
}

int
shell_call(int n, const char *command, ...)
{
    errno = 0;
    pid_t childPid;
    int status = 0, i = 0;
    char com[512];
    
    sprintf(com, "%s", command);
    
    va_list arg_ptr;
    va_start(arg_ptr, command);
    for (i = 0; i < n; i++) {
        strcat(com, " \"");
        strcat(com, va_arg(arg_ptr, char *));
        strcat(com, "\"");
    }
    va_end(arg_ptr);
    
    install_deb("Run command: \n%s\n", com);

    switch (childPid = fork()) {
        case -1:
            extErr("Fork wrong when exclude %s\n", com);
            break;
        case 0:
            execlp("sh", "sh", "-c", com, (char *)0);
            switch (errno) {
                case EACCES:
                    puts("eaccess");
                    break;
                case ENOENT:
                    puts("文件不存在");
                    break;
                case ENOEXEC:
                    puts("无法执行指定文件");
                    break;
                default:
                    puts("Wrong!");
            }
            _exit(-127);
        default:
            break;
    }
    while (waitpid(childPid, &status, 0) == -1) {
        perror("waitpid wrong!");
        if (errno == EINTR) {
            status = -5;
            break;
        }
    }
    if (WIFEXITED(status)) {
        // 子进程正常结束
        // printf("子进程执行成功\n");
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        // 子进程因信号被杀掉
        fprintf(stderr, "Child[%d] is killed by signal[%d]\n", childPid, WTERMSIG(status));
        return -1;
    } else if (WIFSTOPPED(status)) {
        // 子进程因信号而停止
        fprintf(stderr, "Child[%d] is stoped by signal[%d]\n", childPid, WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
        fprintf(stderr, "Child[%d] catch signal[%d]\n", childPid, WSTOPSIG(status));
    }

    return status;
}

int analyse_branch(const char *get_branch_info)
{
    int index = 0, new_install_name = 0;
    int return_code = 0;
    char *get_branch_info_bk = strdup(get_branch_info);
    char *cur_branch_info = strtok(get_branch_info_bk, " ");
    
    if (atoi_install_opt.install_name == NULL) {
        new_install_name = 1;
        atoi_install_opt.install_name = (char *)malloc(sizeof(char) * 40);
        strcpy(atoi_install_opt.install_name, "br_");
    }
    
    while (cur_branch_info != NULL) {
        _atoi_branch_info_ *new_node, *current;
        new_node = (_atoi_branch_info_*)atoi_malloc(sizeof(_atoi_branch_info_));
        
        char *branch_name = NULL;
        if ((branch_name = strchr(cur_branch_info, ':')) != NULL) {
            strcpy(new_node->branch_name, branch_name + 1);
            cur_branch_info[strlen(cur_branch_info) - strlen(branch_name)] = '\0';
            strcpy(new_node->remote, cur_branch_info);
        } else {
            strcpy(new_node->branch_name, cur_branch_info);
            strcpy(new_node->remote, atoi_install_opt.def_base_user);
        }
        install_deb("remote: [%s]; branch: [%s]\n", new_node->remote, new_node->branch_name);
        
        if (!new_node->branch_name || !new_node->remote) {
            return_code = -1;
            goto ERR;
        }
        new_node->index = index;
//        snprintf(atoi_install_opt.install_name, 40, "%s", new_node->branch_name);
        if (new_install_name == 1) {
            strncat(atoi_install_opt.install_name, new_node->branch_name, 40);
        }
        
        new_node->next = NULL;
        
        if (atoi_install_opt.branch_info == NULL) {
            atoi_install_opt.branch_info = new_node;
            current = new_node;
        } else {
            current->next = new_node;
            current = new_node;
        }
        cur_branch_info = strtok(NULL, " ");
        
        index++;
    }
ERR:
    free(get_branch_info_bk);
    return return_code;
}
