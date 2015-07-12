#include <libconfig.h>
#include <getopt.h>
#include "common.h"
#include "atoi_install.h"

enum config_type {
    ATOI_CON_STRING = 1,
    STOI_CON_INT,
    STOI_CON_INT64,
    STOI_CON_FLOAT,
    STOI_CON_BOOL,
};

_atoi_err_mes atoi_err_mes;
struct install_options atoi_install_opt = {
    .install_method = 0,
    .debug = 0,
    .dataloader = 1,
    .avl = 0,
    .unittest = 0,
    .build_code_or_not = 1,
    .init_db_or_not = 1,
    .cp_dataloader = 0,
};

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
static int
atoi_config_lookup_config(config_t *config, int types, const char *key, void *val);

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

static int
atoi_config_lookup_config(config_t *config, int types, const char *key, void *val)
{
    atoi_err_mes.code = 0;
    int config_return = 0;
    
    if (*(char**)val != NULL) {
        return config_return;
    }
    
    switch (types) {
        case ATOI_CON_STRING:
            config_return = config_lookup_string(config, key, (const char **)val);
            break;
        case STOI_CON_INT:
            config_return = config_lookup_int(config, key, (int *)val);
            break;
        case STOI_CON_INT64:
            config_return = config_lookup_int64(config, key, (long long *)val);
            break;
        case STOI_CON_FLOAT:
            config_return = config_lookup_float(config, key, (double *)val);
            break;
        case STOI_CON_BOOL:
            config_return = config_lookup_bool(config, key, (int *)val);
            break;
        default:
            atoi_err_mes.code = -1;
            snprintf(atoi_err_mes.err_mes, 127, "Can not read type [%d] from config file.", types);
            return atoi_err_mes.code;
            break;
    }
    
    if (config_return == CONFIG_FALSE) {
        switch (config_error_type(config)) {
            case CONFIG_ERR_NONE:
                atoi_err_mes.code = -1;
                snprintf(atoi_err_mes.err_mes, 127, "Can not found node [%s] from config file.", key);
                break;
            case CONFIG_ERR_FILE_IO:
                atoi_err_mes.code = -1;
                snprintf(atoi_err_mes.err_mes, 127, "config err file io.");
                break;
            case CONFIG_ERR_PARSE:
                atoi_err_mes.code = -1;
                snprintf(atoi_err_mes.err_mes, 127, "config err parse.");
                break;
            default:
                atoi_err_mes.code = -1;
                snprintf(atoi_err_mes.err_mes, 127, "Unknow config error.");
                break;
        }
    }
    return atoi_err_mes.code;
}

static void
getConfig(struct install_options *opt) {
    const char *git_path            = getenv("ATOI_GIT_PATH");
    const char *build_path          = getenv("ATOI_BUILD_PATH");
    const char *web_path            = getenv("ATOI_WEB_PATH");
    const char *install_hook_script = getenv("ATOI_INSTALL_HOOK_SCRIPT");
    const char *init_db_script      = getenv("ATOI_INIT_DB_SCRIPT");
    const char *def_base_user       = NULL;
    const char *def_head_user       = NULL;
    
    const char *sc_license          = getenv("ATOI_SC_LICENSE");
    const char *web_host            = getenv("ATOI_WEB_HOST");
    const char *sc_admin            = getenv("ATOI_SC_ADMIN");
    const char *sc_admin_pwd        = getenv("ATOI_SC_ADMIN_PWD");
    
#ifdef SERVERINSTALL
    const char *dbname              = getenv("");
    const char *db_port             = getenv("DB_PORT");
    const char *db_host             = getenv("DB_HOST");
    const char *db_admin            = getenv("DB_USER");
    const char *db_admin_pwd        = getenv("DB_PASSWORD");
#else
    const char *dbname              = getenv("ATOI_DBNAME");
    const char *db_port             = getenv("ATOI_DB_PORT");
    const char *db_host             = getenv("ATOI_DB_HOST");
    const char *db_admin            = getenv("ATOI_DB_ADMIN");
    const char *db_admin_pwd        = getenv("ATOI_DB_ADMIN_PWD");
#endif
    
    const char *fts_type            = getenv("ATOI_FTS_TYPE");
    const char *fts_host            = getenv("ATOI_FTS_HOST");
    const char *fts_port            = getenv("ATOI_FTS_PORT");
    
    const char *atoi_tmp_path       = getenv("ATOI_TMP_PATH");
    
    struct config_t config;
    
    /*
     * 初始化
     * void config_init (config_t * config)
     */
    config_init(&config);
    
    /*
     * 从指定的文件中读取配置信息
     * int config_read_file (config_t * config, const char * filename)
     *
     * 以上函数在执行成功后返回: CONFIG_TRUE,
     * 否则返回: CONFIG_FALSE, 并可通过以下宏获取错误信息
     *     const char * config_error_text (const config_t * config)
     *     const char * config_error_file (const config_t * config)
     *     int config_error_line (const config_t * config)
     *     上述这些错误宏返回的字符串, 会在执行 config_destroy() 函数后自动被释放, 不能手动释放这些函数.
     */
    if (config_read_file(&config, "/document/gbyukg/www/test/newInstall/newInstall/config.cfg") == CONFIG_FALSE)
        extErr("%s\n", config_error_text(&config));
    
    // install config
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION ".atoi_git_path",
                                          &git_path));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_web_path",
                                          &web_path));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_build_path",
                                          &build_path));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_install_hook_script",
                                          &install_hook_script));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_init_db_script",
                                          &init_db_script));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_def_base_user",
                                          &def_base_user));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_def_head_user",
                                          &def_head_user));

    // db config
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_default_dbname",
                                          &dbname));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_db_port",
                                          &db_port));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_db_host",
                                          &db_host));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_db_admin",
                                          &db_admin));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_db_admin_pwd",
                                          &db_admin_pwd));
    
    // fts config
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_fts_type",
                                          &fts_type));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_fts_host",
                                          &fts_host));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_fts_port",
                                          &fts_port));
    
    // sc config
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_sc_license",
                                          &sc_license));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_web_host",
                                          &web_host));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_sc_admin",
                                          &sc_admin));
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_sc_admin_pwd",
                                          &sc_admin_pwd));
    
    CUS_SYSCALL(atoi_config_lookup_config(&config,
                                          ATOI_CON_STRING,
                                          VERSION".atoi_tmp_path",
                                          &atoi_tmp_path));
    // install config
    atoi_install_opt.git_path            = strdup(git_path);
    atoi_install_opt.web_path            = strdup(web_path);
    atoi_install_opt.build_path          = strdup(build_path);
    atoi_install_opt.install_hook_script = strdup(install_hook_script);
    atoi_install_opt.init_db_script      = strdup(init_db_script);
    atoi_install_opt.def_base_user       = strdup(def_base_user);
    atoi_install_opt.def_head_user       = strdup(def_head_user);
    
    // sc config
    atoi_install_opt.sc_license = strdup(sc_license);
    atoi_install_opt.web_host = strdup(web_host);
    atoi_install_opt.sc_admin = strdup(sc_admin);
    atoi_install_opt.sc_admin_pwd = strdup(sc_admin_pwd);
    
    // db config
    atoi_install_opt.dbname  = strdup(dbname);
    atoi_install_opt.db_port = strdup(db_port);
    atoi_install_opt.db_host = strdup(db_host);
    atoi_install_opt.db_admin = strdup(db_admin);
    atoi_install_opt.db_admin_pwd = strdup(db_admin_pwd);
    
    // fts config
    atoi_install_opt.fts_type = strdup(fts_type);
    atoi_install_opt.fts_host = strdup(fts_host);
    atoi_install_opt.fts_port = strdup(fts_port);
    
    atoi_install_opt.tmp_path = strdup(atoi_tmp_path);
    
    const char *home_dir = NULL;
    if ((home_dir = getenv("HOME")) != NULL) {
        atoi_install_opt.home_dir = strdup(getenv("HOME"));
    } else {
        extErr("get home dir wrong! [getenv(\"HOME\")]\n");
    }
    
    /*
     * 清除
     * void config_init (config_t * config)
     * 会释放为字符串分配的内存空间
     */
    config_destroy(&config);
}

static void
init_install() {
    install_deb("init install ...\n");
    int len = sizeof(char) * 1024;
    atoi_install_opt.cur_dir = atoi_malloc(len);

    if (getcwd(atoi_install_opt.cur_dir, len) == NULL)
        extErr("获取程序当前执行路径失败: %s.\n", strerror(errno));
    
    install_deb("Current directory: %s\n", atoi_install_opt.cur_dir);
    
#ifdef SERVERINSTALL
    atoi_install_opt.git_path = getenv("ATOI_GIT_PATH");
    atoi_install_opt.web_path = getenv("ATOI_WEB_PATH");
    atoi_install_opt.build_path = getenv("ATOI_BUILD_PATH");
    atoi_install_opt.install_hook_script = getenv("ATOI_INSTALL_HOOK_SCRIPT");
    atoi_install_opt.init_db_script = getenv("ATOI_INIT_DB_SCRIPT");
    atoi_install_opt.def_base_user = getenv("ATOI_DEF_BASE_USER");
    atoi_install_opt.def_head_user = getenv("ATOI_DEF_HEAD_USER");

    atoi_install_opt.sc_license = getenv("ATOI_SC_LICENSE");
    atoi_install_opt.web_host = getenv("ATOI_WEB_HOST");
    atoi_install_opt.sc_admin = getenv("ATOI_SC_ADMIN");
    atoi_install_opt.sc_admin_pwd = getenv("ATOI_SC_ADMIN_PWD");

    atoi_install_opt.dbname  = getenv("DB_NAME");
    atoi_install_opt.db_port = getenv("DB_PORT");
    atoi_install_opt.db_host = getenv("DB_HOST");
    atoi_install_opt.db_admin = getenv("DB_USER");
    atoi_install_opt.db_admin_pwd = getenv("DB_PASSWORD");

    atoi_install_opt.fts_type = getenv("ATOI_FTS_TYPE");
    atoi_install_opt.fts_host = getenv("ATOI_FTS_HOST");
    atoi_install_opt.fts_port = getenv("ATOI_FTS_PORT");
    
    atoi_install_opt.tmp_path = getenv("ATOI_TMP_PATH");

    atoi_install_opt.home_dir = getenv("HOME");

#else
    getConfig(&atoi_install_opt);
#endif
    
    if (install_hook("init", NULL) != 0)
        extErr("Run hook [init] wrong!\n");

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

// gcc -g `pkg-config --cflags --libs libconfig` -lcurl -lyajl -I/usr/local/Cellar/libgit2/libgit2-0.22.3/include -L/usr/local/Cellar/libgit2/libgit2-0.22.3/build/ -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c && ./a.out -b 17684

// gcc -g -lconfig -lyajl -lgit2 common.c atoi_curl.c atoi_install.c atoi_git.c main.c -o atoi-install -DSERVERINSTALL && ./atoi-install install
int main(int argc, const char * argv[])
{
    if (argc < 2) {
        usage();
        exit(1);
    }
    
    // 系统初始化
    init_install();

    int i = 1;
    if (strcmp("install", argv[i]) == 0) {
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
        
        while ((opt = getopt_long(argc, (char *const*)argv, short_opt, long_options, &indexptr)) != -1) {
            switch (opt) {
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
        
        if (!atoi_install_opt.install_method) {
            install_usage();
            exit(-1);
        }
        
    } else if (strcmp("repair", argv[i]) == 0) {
        
    } else if (strcmp("create-pr", argv[i]) == 0) {
        
    } else if (strcmp("format-js", argv[i]) == 0) {
        
    } else {
        usage();
        abort();
    }
    
    switch (atoi_install_opt.install_method) {
        case BRANCH_INSTALL:
            branch_install();
            
            // 资源释放
            _atoi_branch_info_ *relast = atoi_install_opt.branch_info;
            while (relast != NULL) {
                install_deb("Free branch source...\n");
                _atoi_branch_info_ *relast_next = relast->next;
                free(relast);
                relast = relast_next;
            }
            free(atoi_install_opt.install_name);
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
            if (atoi_install_opt.install_name == NULL) {
                atoi_install_opt.install_name = strdup(get_cur_branch_name());
            }
            install_deb("Install name: [%s]\n", atoi_install_opt.install_name);
                        install_mes("Install from current git repository branch [%s]\n",
                                    atoi_install_opt.install_name);
            start_install();
            free(atoi_install_opt.install_name);
            break;
        case WEB_DIR_INSTALL:
            // check dir exist
            
            start_install();
            free(atoi_install_opt.install_name);
            break;
        case PACKAGE_INSTALL:
            // install from package
            package_install(atoi_install_opt.package_info);
            
            free(atoi_install_opt.package_info);
            free(atoi_install_opt.install_name);
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
            break;
    }

    free(atoi_install_opt.dbname);
    free(atoi_install_opt.git_path);
    free(atoi_install_opt.web_path);
    free(atoi_install_opt.build_path);
    free(atoi_install_opt.def_base_user);
    free(atoi_install_opt.def_head_user);
    free(atoi_install_opt.cur_dir);
    free(atoi_install_opt.home_dir);

    return 0;
}
