//
//  atoi_curl.c
//  newInstall
//
//  Created by 张贺 on 15-5-28.
//  Copyright (c) 2015年 张贺. All rights reserved.
//

#include "atoi_curl.h"

extern struct install_options atoi_install_opt;
//static CURL *curl = NULL;

static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size);

static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size)
{
    size_t i;
    size_t c;
    unsigned int width=0x10;
    
    fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
            text, (long)size, (long)size);
    
    for(i=0; i<size; i+= width) {
        fprintf(stream, "%4.4lx: ", (long)i);
        
        /* show hex to the left */
        for(c = 0; c < width; c++) {
            if(i+c < size)
                fprintf(stream, "%02x ", ptr[i+c]);
            else
                fputs("   ", stream);
        }
        
        /* show data on the right */
        for(c = 0; (c < width) && (i+c < size); c++)
            fputc((ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.', stream);
        
        fputc('\n', stream); /* newline */
    }
}

int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp) {
    const char *text;
    (void)handle;
    
    switch(type) {
        case CURLINFO_TEXT:
            fprintf(stderr, "== Info: %s", data);
        default:
            return 0;
        case CURLINFO_HEADER_OUT:
            text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            text = "<= Recv data";
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }
    dump (text, stderr, (unsigned char *)data, size);
    return 0;
}

CURL *atoi_get_curl(const char *url, struct curl_slist *headers)
{
//    struct curl_slist *headers = NULL;

    CURL *curl = curl_easy_init();
    
    if (!curl) {
        extErr("CURL 初始化失败");
    }
    
    // 设置错误信息保存位置
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_err_buf);
    
    if (atoi_install_opt.debug) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
    }
    
    // 如果开启头文件输出信息, 头信息也会被 write_callback 获取到
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    
    if (atoi_install_opt.debug) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
    }

    headers = curl_slist_append(headers, "User-Agent:gbyukg");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    return curl;
}

int atoi_create_pull_request()
{
    int err = 0;
    git_repository *repo = NULL;
    CURL *curl = NULL;
    git_oid head_oid;
    git_reference *head_ref = NULL;
    const char *head_ref_name = NULL;
    struct curl_slist *headers = NULL;
    git_commit *head_commit = NULL;
    char *commit_message = NULL;
    char post_data[1024];
    const char *url = "https://api.github.com/repos/sugareps/Mango/pulls";
    
    install_deb("基准分支: [%s]\n", atoi_install_opt.cr_pr->bash_ref);
    atoi_git_init(atoi_install_opt.git_path, &repo);
    
    if (atoi_install_opt.cr_pr->head_ref == NULL) {
        get_current_branch(&atoi_install_opt.cr_pr->head_ref, repo);
        install_deb("获取当前分支: [%s]\n", atoi_install_opt.cr_pr->head_ref);
    }
    
    install_mes("Creat pull request into [%s] from [%s]\n",
                atoi_install_opt.cr_pr->bash_ref,
                atoi_install_opt.cr_pr->head_ref);
    
    git_err(git_branch_lookup(&head_ref, repo, atoi_install_opt.cr_pr->head_ref, GIT_BRANCH_LOCAL));
    if ((head_ref_name = git_reference_name(head_ref)) == NULL)
        extErr("head_ref_name is empty");
    git_err(git_reference_name_to_id(&head_oid, repo, head_ref_name));
    git_err(git_commit_lookup(&head_commit, repo, &head_oid));
    if ((commit_message = (char*)git_commit_message(head_commit)) == NULL)
        extErr("Commit message is NULL");
    // 去掉commit信息最后一个换行符
    size_t mes_len = strlen(commit_message);
    if (commit_message[mes_len - 1] == '\n') {
        commit_message[mes_len - 1] = '\0';
    }
    
    char toke[62] = "Authorization: token ";
    if (atoi_install_opt.token == NULL) {
        extErr("token is null.\n");
    }
    strcat(toke, atoi_install_opt.token);
    headers = curl_slist_append(headers, toke);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl = atoi_get_curl(url, headers);
    
    sprintf(post_data, "{\"title\":\"%s\",\"head\":\"%s:%s\",\"base\":\"%s\",\"body\":\"%s\"}",
                                commit_message,
                                "gbyukg",
                                atoi_install_opt.cr_pr->head_ref,
                                atoi_install_opt.cr_pr->bash_ref,
                                commit_message);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    
    if (CURLE_OK != curl_easy_perform(curl)) {
        extErr("curl_easy_perfomr wrong message: %s", curl_err_buf);
    }
    
    curl_slist_free_all(headers);
    git_repository_free(repo);
    return err;
}