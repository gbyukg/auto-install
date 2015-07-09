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

CURL *atoi_get_curl(const char *url, struct curl_slist **headers)
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

    *headers = curl_slist_append(*headers, "User-Agent:gbyukg");
    
//    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    return curl;
}