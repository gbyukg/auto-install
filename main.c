#include <getopt.h>
#include "common.h"
#include "atoi_install.h"
#include "iniparser.h"

enum config_type
{
    ATOI_CON_STRING = 1,
    STOI_CON_INT,
    STOI_CON_INT64,
    STOI_CON_FLOAT,
    STOI_CON_BOOL,
};

_atoi_err_mes atoi_err_mes;
struct install_options atoi_install_opt =
{
    .install_method = 0,
    .debug = 0,
    .dataloader = 1,
    .avl = 0,
    .unittest = 0,
    .build_code_or_not = 1,
    .init_db_or_not = 1,
    .cp_dataloader = 0,
    .run_after_install = 1,
    .update_composer = 1,
};

//extern FILE *log_file;

static void
open_log();

/*
 * 系统初始化
 */
static void
init_install(void);

/*
 * 获取配置文件信息
 * @install_options: 保存安装配置信息
 */
static void
getConfig(struct install_options*);

/*
 * 从配置文件中读取配置信息
 * @config: struct config_t config.
 *          由 config_init 初始化而来
 * @types:  string, int, int64, float, bool, string.
 *          要读取的配置的类型
 * @key:    配置文件中的键值
 * @val:    保存获取到的配置文件中的值
 */
//static int
//atoi_config_lookup_config(config_t *config, int types, const char *key, void *val);

static void usage(void)
{
    fprintf(stderr,
            "atoi [option]... \n"
            "  -p|--pull <pr>           根据pull request号<pr>安装.\n"
            "  -b|--branch <br1;br2...> 安装指定的分支<br>.\n"
            "  -d|--database            指定使用的数据库名称(默认为sugarcrm).\n"
            "  -v|--verbose             输出详细安装过程.\n"
            "  --gitpath                GIT 路径.\n"
            "  --webpath                WEB 路径.\n"
            "  -?|-h|--help             This information.\n"
            "  -V|--version             Display program version.\n"
           );
};

static const
char *atoi_iniparser_getstring(dictionary *ini, const char *key, const char *def_val)
{
    if (def_val != NULL)
    {
        return strdup(def_val);
    }
    char final_key[256] = VERSION ":";
    strncat(final_key, key, 200);

    return strdup(iniparser_getstring(ini, final_key, def_val));
}

static void
getConfig(struct install_options *opt)
{
    dictionary *ini ;

    // 读取 HOME 环境变量, 赋值给 home_dir
    const char *home_dir = NULL;
    if ((home_dir = getenv("HOME")) != NULL)
    {
        atoi_install_opt.home_dir = strdup(getenv("HOME"));
    }
    else
    {
        extErr("get home dir wrong! [getenv(\"HOME\")]");
    }

    // 读取配置文件
    const char *ini_name = NULL;
    if ((ini_name = getenv("ATOI_CONFIG_PATH")) == NULL)
        extErr("Config file path is NULL!");

//    const char *ini_name = "/document/gbyukg/www/test/newInstall/newInstall/config.ini";
    if ((ini = iniparser_load(ini_name)) == NULL)
        extErr("Cannot parse file: %s", ini_name);

    if (atoi_install_opt.debug == 1)
    {
        iniparser_dump(ini, stderr);
    }

    const char *git_path            = getenv("ATOI_GIT_PATH");
    const char *build_path          = getenv("ATOI_BUILD_PATH");
    const char *web_path            = getenv("ATOI_WEB_PATH");
    const char *install_hook_script = getenv("ATOI_INSTALL_HOOK_SCRIPT");
    const char *init_db_script      = getenv("ATOI_INIT_DB_SCRIPT");
    const char *token               = getenv("ATOI_token");
    const char *def_base_user       = NULL;
    const char *def_head_user       = NULL;

    const char *sc_license          = getenv("ATOI_SC_LICENSE");
    const char *web_host            = getenv("ATOI_WEB_HOST");
    const char *sc_admin            = getenv("ATOI_SC_ADMIN");
    const char *sc_admin_pwd        = getenv("ATOI_SC_ADMIN_PWD");


    const char *dbname              = getenv("ATOI_DBNAME");
    const char *db_port             = getenv("ATOI_DB_PORT");
    const char *db_host             = getenv("ATOI_DB_HOST");
    const char *db_admin            = getenv("ATOI_DB_ADMIN");
    const char *db_admin_pwd        = getenv("ATOI_DB_ADMIN_PWD");

    const char *fts_type            = getenv("ATOI_FTS_TYPE");
    const char *fts_host            = getenv("ATOI_FTS_HOST");
    const char *fts_port            = getenv("ATOI_FTS_PORT");

    const char *atoi_tmp_path       = getenv("ATOI_TMP_PATH");

    atoi_install_opt.git_path            = atoi_iniparser_getstring(ini, "atoi_git_path", git_path);
    atoi_install_opt.web_path            = atoi_iniparser_getstring(ini, "atoi_web_path", web_path);
    atoi_install_opt.build_path          = atoi_iniparser_getstring(ini, "atoi_build_path", build_path);
    atoi_install_opt.install_hook_script = atoi_iniparser_getstring(ini, "atoi_install_hook_script", install_hook_script);
    atoi_install_opt.init_db_script      = atoi_iniparser_getstring(ini, "atoi_init_db_script", init_db_script);
    atoi_install_opt.token               = atoi_iniparser_getstring(ini, "atoi_token", token);
    atoi_install_opt.def_base_user       = atoi_iniparser_getstring(ini, "atoi_def_base_user", def_base_user);
    atoi_install_opt.def_head_user       = atoi_iniparser_getstring(ini, "atoi_def_head_user", def_head_user);

    // sc config
    atoi_install_opt.sc_license          = atoi_iniparser_getstring(ini, "atoi_sc_license", sc_license);
    atoi_install_opt.web_host            = atoi_iniparser_getstring(ini, "atoi_web_host", web_host);
    atoi_install_opt.sc_admin            = atoi_iniparser_getstring(ini, "atoi_sc_admin", sc_admin);
    atoi_install_opt.sc_admin_pwd        = atoi_iniparser_getstring(ini, "atoi_sc_admin_pwd", sc_admin_pwd);

    // db config
    atoi_install_opt.dbname              = atoi_iniparser_getstring(ini, "atoi_db_name", dbname);
    atoi_install_opt.db_port             = atoi_iniparser_getstring(ini, "atoi_db_port", db_port);
    atoi_install_opt.db_host             = atoi_iniparser_getstring(ini, "atoi_db_host", db_host);
    atoi_install_opt.db_admin            = atoi_iniparser_getstring(ini, "atoi_db_admin", db_admin);
    atoi_install_opt.db_admin_pwd        = atoi_iniparser_getstring(ini, "atoi_db_admin_pwd", db_admin_pwd);

    // fts config
    atoi_install_opt.fts_type            = atoi_iniparser_getstring(ini, "atoi_fts_type", fts_type);
    atoi_install_opt.fts_host            = atoi_iniparser_getstring(ini, "atoi_fts_host", fts_host);
    atoi_install_opt.fts_port            = atoi_iniparser_getstring(ini, "atoi_fts_port", fts_port);

    atoi_install_opt.tmp_path            = atoi_iniparser_getstring(ini, "atoi_tmp_path", atoi_tmp_path);

    // 释放资源
    iniparser_freedict(ini);
}

static void
open_log()
{
    char log_file_name[256];
    snprintf(log_file_name, 255, "%s%s_%d", atoi_install_opt.tmp_path, atoi_install_opt.install_name ,getpid());

    if ((log_file = fopen(log_file_name, "w+")) == NULL) {
        perror("fone wrong!");
        exit(EXIT_FAILURE);
    }
}

static void
init_install()
{
    install_deb("init install ...\n");
    int len = sizeof(char) * 1024;
    atoi_install_opt.cur_dir = atoi_malloc(len);

    if (getcwd(atoi_install_opt.cur_dir, len) == NULL)
        extErr("获取程序当前执行路径失败: %s.", strerror(errno));

    install_deb("Current directory: %s\n", atoi_install_opt.cur_dir);

    // 读取配置文件信息
    getConfig(&atoi_install_opt);

    curl_global_init(CURL_GLOBAL_ALL);
}

static void
install_usage(void)
{
    fprintf(stderr,
            "atoi install [option]... \n"
            "  -p|--pull-install <pr>               根据pull request号<pr>安装.\n"
            "  -b|--branch-install <\"br1;br2...\"> 安装指定的分支<br>.\n"
            "  -w|web-install <dir name>            直接从web目录中安装.\n"
            "  -g|git-install                       直接从当前代码库构建代码并安装.\n"
            "  -P|pack-install <\"package url1 url2...\">  \n"
            "                                       下载并安装 url1, url2... 包.\n"
            "  -d|--dbname <dbname>                 指定使用的数据库名称(默认为sugarcrm).\n"
            "  -c|--cp-vendor <vendor dir>          若指定该选项, 则从指定目录拷贝vendor而不是更新vendor.\n"
            "  --build-code                         通过命令 build 代码并复制到 web 目录下(默认).\n"
            "  --no-build-code                      不会build代码, 当使用 web-install 安装时指定该选项.\n"
            "  --init-db                            初始化数据库(默认).\n"
            "  --no-init-db                         不初始化数据库.\n"
            "  --dl                                 加载 dataloader (默认).\n"
            "  --no-dl                              不加载 dataloader.\n"
            "  --avl                                加载 AVL.\n"
            "  --no-avl                             不加载 AVL(默认).\n"
            "  --unit                               安装完成后执行 unit test.\n"
            "  --no-unit                            安装完成后不执行 unit test(默认).\n"
            "  --debug                              开启 debug 模式.\n"
            "  -h|--help                            帮助信息.\n"
            );
}

static void
atoi_usage(void)
{
    fprintf(stderr,
            "  install [option]          安装SC\n"
            "  repair <web_dir>          repair 指定的目录.\n"
            "  cr-pr <bash_head>         将mango目录下的当前分支以 <bash_head> 为基准分支创建pull request.\n"
            "  format-js <web_dir>       格式化指定安装目录下的JS文件"
            "  -h|--help                 帮助信息.\n"
           );
}

// gcc -g `pkg-config --cflags --libs libconfig` -lcurl -lyajl -I/usr/local/Cellar/libgit2/libgit2-0.22.3/include -L/usr/local/Cellar/libgit2/libgit2-0.22.3/build/ -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c && ./a.out -b 17684

// gcc -Wall -g `pkg-config --cflags --libs libconfig` -lcurl -lyajl -I/usr/local/Cellar/libgit2/libgit2-0.23.0/include -L/usr/local/Cellar/libgit2/libgit2-0.23.0/build/ -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-sc && ./atoi-sc install --pull-install 18985 --debug

// gcc -Wall -g  -lcurl -lyajl -I/usr/local/Cellar/libgit2/libgit2-0.23.0/include -L/usr/local/Cellar/libgit2/libgit2-0.23.0/build/ -lgit2 common.c dictionary.c iniparser.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-sc && ./atoi-sc install --pull-install 18985 --debug
int main(int argc, const char * argv[])
{
    if (argc < 2)
    {
        atoi_usage();
        exit(1);
    }

    // 系统初始化
    init_install();

    int i = 1;
    if (strcmp("install", argv[i]) == 0)
    {
        struct option long_options[] =
        {
            {"pull-install",        required_argument, 0, 'p'},
            {"branch-install",      required_argument, 0, 'b'},
            /* 直接安装 web 目录安装 */
            {"web-install",         required_argument, 0, 'w'},
            {"git-install",         no_argument,       0, 'g'},
            {"pack-install",        required_argument, 0, 'P'},
            {"dbname",              required_argument, 0, 'd'},
            {"install-name",        required_argument, 0, 'n'},
            {"cp-vendor",           required_argument, 0, 'c'},
            {"help",                no_argument,       0, 'h'},
            {"cp-dataloader",       no_argument, &atoi_install_opt.cp_dataloader, 1},
            {"build-code",          no_argument, &atoi_install_opt.build_code_or_not, 1},
            {"no-build-code",       no_argument, &atoi_install_opt.build_code_or_not, 0},
            {"init-db",             no_argument, &atoi_install_opt.init_db_or_not, 1},
            {"no-init-db",          no_argument, &atoi_install_opt.init_db_or_not, 0},
            {"dl",                  no_argument, &atoi_install_opt.dataloader, 1},
            {"no-dl",               no_argument, &atoi_install_opt.dataloader, 0},
            {"avl",                 no_argument, &atoi_install_opt.avl, 1},
            {"no-avl",              no_argument, &atoi_install_opt.avl, 0},
            {"unit",                no_argument, &atoi_install_opt.unittest, 1},
            {"no-unit",             no_argument, &atoi_install_opt.unittest, 0},
            {"debug",               no_argument, &atoi_install_opt.debug, 1},
            {0, 0, 0, 0}
        };
        char *const short_opt = "p:P:b:w:gd:hn:";
        int opt, indexptr;

        while ((opt = getopt_long(argc, (char *const*)argv, short_opt, long_options, &indexptr)) != -1)
        {
            switch (opt)
            {
            case 'p':
                if (atoi_install_opt.install_method)
                    break;
                atoi_install_opt.install_method = PULL_INSTALL;
                atoi_install_opt.pull_info = (_atoi_pull_info_*)atoi_malloc(sizeof(_atoi_pull_info_));
                atoi_install_opt.pull_info->pull_number = strndup(optarg, strlen(optarg));
                break;
            case 'b':
                if (atoi_install_opt.install_method)
                    break;
                atoi_install_opt.install_method = BRANCH_INSTALL;
                install_deb("Analyse branch info: [%s]\n", optarg);
                SYSCALL(analyse_branch(optarg));
                break;
            case 'g':
                if (atoi_install_opt.install_method)
                    break;
                atoi_install_opt.install_method = GIT_INSTALL;
                break;
            case 'w':
                if (atoi_install_opt.install_method)
                    break;
                atoi_install_opt.install_method = WEB_DIR_INSTALL;
                atoi_install_opt.build_code_or_not = 0;
                atoi_install_opt.install_name = strndup(optarg, strlen(optarg));
                break;
            case 'P':
                if (atoi_install_opt.install_method)
                    break;
                atoi_install_opt.install_method = PACKAGE_INSTALL;
                atoi_install_opt.package_info = strndup(optarg, strlen(optarg));
                break;
            case 'c':

                break;
            case 'd':
                atoi_install_opt.dbname = strdup(optarg);
                break;
            case 'n':
                atoi_install_opt.install_name = strdup(optarg);
                break;
            case 'h':
            case '?':
                install_usage();
                exit(0);
                break;
            default:
                break;
            }
        }

        if (!atoi_install_opt.install_method)
        {
            install_usage();
            exit(-1);
        }
        
        // 打开日志文件
        open_log();
    }
    else if (strcmp("repair", argv[i]) == 0)
    {

    }
    else if (strcmp("cr-pr", argv[i]) == 0)
    {
        if (argc < 2) {
            extErr("需要指定基准分支");
        }
        atoi_install_opt.cr_pr = (_atoi_cr_pr_*)atoi_malloc(sizeof(_atoi_cr_pr_));
        atoi_install_opt.cr_pr->head_ref = NULL;
        atoi_install_opt.cr_pr->bash_ref = strdup(argv[2]);
        atoi_install_opt.install_method = CR_PR;
    }
    else if (strcmp("format-js", argv[i]) == 0)
    {

    }
    else
    {
        usage();
        abort();
    }

    // 执行初始化脚本钩子
    if (install_hook("init", NULL) != 0)
        extErr("Run hook [init] wrong!");
    
    switch (atoi_install_opt.install_method)
    {
    case BRANCH_INSTALL:
        branch_install();

        // 资源释放
        _atoi_branch_info_ *relast = atoi_install_opt.branch_info;
        while (relast != NULL)
        {
            install_deb("Free branch source...\n");
            _atoi_branch_info_ *relast_next = relast->next;
            free(relast);
            relast = relast_next;
        }
        free((void *)atoi_install_opt.install_name);
        break;
    case PULL_INSTALL:
        pull_install();

#ifndef SERVERINSTALL
        // 资源释放
        free(atoi_install_opt.pull_info->pull_number);
        free(atoi_install_opt.pull_info->diff_url);
        free(atoi_install_opt.pull_info->patch_url);
        free(atoi_install_opt.pull_info->state);
        free(atoi_install_opt.pull_info->title);
        free(atoi_install_opt.pull_info->body);
        free(atoi_install_opt.pull_info->user_login);
        free(atoi_install_opt.pull_info->head_ref);
        free(atoi_install_opt.pull_info->head_user_login);
        free(atoi_install_opt.pull_info->head_repo_name);
        free(atoi_install_opt.pull_info->head_repo_ssh_url);
        free(atoi_install_opt.pull_info->base_ref);
        free(atoi_install_opt.pull_info->base_user_login);
        free(atoi_install_opt.pull_info->base_repo_name);
        free(atoi_install_opt.pull_info->base_repo_ssh_url);
        free(atoi_install_opt.pull_info);
#endif
        break;
    case GIT_INSTALL:
        if (atoi_install_opt.install_name == NULL)
        {
            atoi_install_opt.install_name = strdup(get_cur_branch_name());
        }
        install_deb("Install name: [%s]\n", atoi_install_opt.install_name);
        install_mes("Install from current git repository branch [%s]\n",
                    atoi_install_opt.install_name);
        start_install();
        free((void *)atoi_install_opt.install_name);
        break;
    case WEB_DIR_INSTALL:
        // check dir exist

        start_install();
        free((void *)atoi_install_opt.install_name);
        break;
    case PACKAGE_INSTALL:
        // install from package
        package_install(atoi_install_opt.package_info);

        free((void *)atoi_install_opt.package_info);
        free((void *)atoi_install_opt.install_name);
        break;
        case CR_PR:
            atoi_create_pull_request();
            
            free(atoi_install_opt.cr_pr->bash_ref);
            free(atoi_install_opt.cr_pr);
            break;
    default:
        usage();
        exit(EXIT_FAILURE);
        break;
    }

    free((void *)atoi_install_opt.dbname);
    free((void *)atoi_install_opt.git_path);
    free((void *)atoi_install_opt.web_path);
    free((void *)atoi_install_opt.build_path);
    free((void *)atoi_install_opt.def_base_user);
    free((void *)atoi_install_opt.def_head_user);
    free((void *)atoi_install_opt.cur_dir);
    free((void *)atoi_install_opt.home_dir);
    free((void *)atoi_install_opt.token);

    fclose(log_file);
    
    return 0;
}

