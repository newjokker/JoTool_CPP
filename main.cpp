
#include <iostream>
#include <fstream>
#include <set>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include "include/strToImg.hpp"
#include "include/deteRes.hpp"
#include "include/operateDeteRes.hpp"
#include "include/pystring.h"
#include "include/fileOperateUtil.hpp"
#include "include/ucDatasetUtil.hpp"
#include "include/xini_file.h"
#include "include/saturn_database_sql.hpp"
#include "include/paramInfo.hpp"
#include "include/printCpp.hpp"
#include "include/lablelmeObj.hpp"
#include <nlohmann/json.hpp>
#include "include/lablelmeObj.hpp"
#include "include/tqdm.h"

using json = nlohmann::json;
using namespace jotools;
using namespace std;


// nginx 负载均衡，可以在风火轮上部署，转到 111 和 209 服务器上

// 完成 C++ 版本的 文件 服务，再部署到 docker 上面，这样在哪个服务器上都能方便进行启动

// todo json 读取大文件太慢，能不能只读取需要的部分，比如打印 info 信息，不需要后面的 xml_info 信息，是否可以不读取到内存里，不行的话就直接自己写一个解析 json 的代码，用做快速解析

// 下载某一些图片报错，可能是没有按照各个后缀依次寻找图片

// 交互设计之类的全部抄 git 的

// 解析出来的 json 中存放的 jpg 图片是压缩过的，这样存放的数据能小一点

// 我之前的 python acc 代码有问题，计算正确的个数的时候明显不对，需要修改

// add config such as -c --c 

// add meachine learning , basic content,  

// label_used 是无序的要记得这个

// ucd help 1 , 第一种格式的打印， ucd help 2 第二种格式的打印，有些格式打印出来的要是中文

// python 是不是要搞一套一样的接口的包，方便使用，

// 查看对文件夹是否有操作权限，没有权限的话 save 等操作没有结果但是也不会报错

// 标准的质量控制，去掉那些重复的，可以反复跑训练集，得到结果，进行对比

// 增加寻找有问题的 标签的信息


int main(int argc, char ** argv)
{

    // param
    UcdParamOpt *ucd_param_opt = new UcdParamOpt();
    ucd_param_opt->load_param_info();

    if ((argc < 2))
    {
        std::cout << "need parameter number >= 1 get : " << argc-1 << std::endl;
        ucd_param_opt->print_all_fun();
        return -1;
    }

    // server
    std::string host        = "192.168.3.111";
    int port                = 11101;
    std::string config_path;
    
    // sql info 
    int sql_port = 3306;
    std::string sql_host    = "192.168.3.101";
    std::string sql_user    = "root";
    std::string sql_pwd     = "root123";
    std::string sql_db      = "Saturn_Database_V1";
    
    // version
    std::string app_version = "v1.5.2";

    // cache dir
    std::string cache_dir;

    // get user name
    struct passwd* pwd;
    uid_t userid;
	userid = getuid();
	pwd = getpwuid(userid);
    std::string pw_name = pwd->pw_name;
    
    // if config_path is "~/ucdconfig.ini" can't read the file, so should get the user name for ~
    if(pw_name == "root")
    {
        config_path = "/" + pw_name + "/ucdconfig.ini";
    }
    else
    {
        config_path = "/home/" + pw_name + "/ucdconfig.ini";
    }

    // read config
    if(is_file(config_path))
    {
        xini_file_t xini_file(config_path);
        host = (const std::string &)xini_file["server"]["host"];
        port = (const int &)xini_file["server"]["port"];
        sql_host = (const std::string &)xini_file["sql"]["host"];
        sql_port = (const int &)xini_file["sql"]["port"];
        sql_user = (const std::string &)xini_file["sql"]["user"];
        sql_pwd = (const std::string &)xini_file["sql"]["pwd"];
        sql_db = (const std::string &)xini_file["sql"]["db"];
        cache_dir = (const std::string &)xini_file["cache"]["dir"];
    }
    else
    {
        xini_file_t xini_write(config_path);
        xini_write["server"]["host"] = host;
        xini_write["server"]["port"] = port;
        xini_write["sql"]["host"] = sql_host;
        xini_write["sql"]["port"] = sql_host;
        xini_write["sql"]["user"] = sql_user;
        xini_write["sql"]["pwd"] = sql_pwd;
        xini_write["sql"]["db"] = sql_db;
        xini_write.dump(config_path);   
    }
    
    // init ucd_util
    UCDatasetUtil* ucd_util = new UCDatasetUtil(host ,port, cache_dir);
    std::string command_1 = argv[1];

    // must set ucd_cache 
    if((! is_dir(cache_dir)) && (command_1 != "set"))
    {
        std::cout << "cache_dir not exists, edit ucdconfig.ini cache/cache_dir : " << std::endl;
        std::cout << "ucdconfig path : " << config_path << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << "set cache_dir with 'ucd set cache_dir {cache_dir}' " << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        throw "cache_dir not exists!";
    }

    // all command
    if(command_1 == "check")
    {
        if(argc == 2)
        {
            ucd_util->search_ucd();
        }
        else
        {
            ucd_param_opt->print_command_info("check");
            return -1;
        }
    }
    else if(command_1 == "search")
    {
        if(argc == 2)
        {
            ucd_util->search_ucd();
        }
        else
        {
            ucd_param_opt->print_command_info("check");
            return -1;
        }
    }
    else if(command_1 == "load")
    {        
        if(argc < 2)
        {
            ucd_param_opt->print_command_info("load");
            return -1;
        }
        std::string ucd_name = argv[2];
        std::string ucd_save_path;
        if(argc == 3)
        {
            ucd_save_path = ucd_name + ".json";
        }
        else if (argc == 4)
        {
            ucd_save_path = argv[3];
            if(is_dir(ucd_save_path))
            {
                ucd_save_path += "/" + ucd_name + ".json";
            }
        }
        else
        {
            ucd_param_opt->print_command_info("load");
            return -1;
        }
        ucd_util->load_ucd(ucd_name, ucd_save_path);
    }
    else if(command_1 == "delete")
    {
        if(argc != 3)
        {
            ucd_param_opt->print_command_info("delete");
            return -1;
        }
        else
        {
            std::string std_name = argv[2];
            ucd_util->delete_ucd(std_name);
        }
    }
    else if(command_1 == "save")
    {
        if((argc == 5) || (argc == 6))
        {
            std::string ucd_path = argv[2];
            std::string save_dir = argv[3];
            std::string save_mode = argv[4];
            
            if(! is_file(ucd_path))
            {
                std::cout << "json_path not exists : " << ucd_path << std::endl;
                throw "json_path not exists";
            }

           if(! is_dir(save_dir))
            {
                std::cout << "save_dir not exists : " << save_dir << std::endl;
                throw "save_dir not exists";
             }

            if(save_mode.size() != 2)
            {
                std::cout << "save_mode illeagal, need save_mode such as 11 | 10  " << save_dir << std::endl;
                throw "save_mode illeagal";
            }

            bool need_img, need_xml;
            if(save_mode[0] == '0')
            {
                need_img = false;
            }
            else
            {
                need_img = true;
            }

            if(save_mode[1] == '0')
            {
                need_xml = false;
            }
            else
            {
                need_xml = true;
            }

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd();

            int need_count = -1;
            std::vector<std::string> uc_vector;
            if(argc == 6)
            {
                need_count = std::stoi(argv[5]);
                uc_vector = ucd->uc_slice(0, need_count);
            }
            else
            {
                uc_vector = ucd->uc_list;
            }

            if(need_img){ucd_util->load_img(save_dir, uc_vector);}
            if(need_xml){ucd_util->load_xml(save_dir, uc_vector);}
            delete ucd;
        }
        else
        {
            ucd_param_opt->print_command_info("save");
            return -1;
        }
    }
    else if(command_1 == "save_cache")
    {
        // 保存到缓存中，xml 和 img 都有对应的缓存

        if(argc == 4)
        {
            std::string json_path = argv[2];
            std::string save_mode = argv[3];
            
            // json_path
            if(! is_file(json_path))
            {
                std::cout << "json_path not exists : " << json_path << std::endl;
                throw "json_path not exists";
            }

            // save_mode
            if(save_mode.size() != 2)
            {
                std::cout << "save_mode illeagal, need save_mode such as 11 | 10  " << std::endl;
                throw "save_mode illeagal";
            }

            // load 
            bool need_img, need_xml;
            // std::string save_img_dir = ucd_util->cache_img_dir;
            // std::string save_xml_dir = 
        
            if(save_mode[0] == '0')
            {
                need_img = false;
            }
            else
            {
                need_img = true;
            }

            if(save_mode[1] == '0')
            {
                need_xml = false;
            }
            else
            {
                need_xml = true;
            }

            // load
            UCDataset* ucd = new UCDataset(json_path);
            ucd->parse_ucd();
            // ucd_util->json_path = json_path;
            std::vector<std::string> uc_vector = ucd->uc_list;
            // 
            if(need_img){ucd_util->load_img(ucd_util->cache_img_dir, uc_vector);}
            if(need_xml){ucd_util->load_xml(ucd_util->cache_xml_dir, uc_vector);}
            delete ucd;
        }
        else
        {
            ucd_param_opt->print_command_info("save_cache");
        }

    }
    else if(command_1 == "upload")
    {
        std::string ucd_path, assign_ucd_name;
        if(argc == 3)
        {
            ucd_path = argv[2];
            ucd_util->upload_ucd(ucd_path);
        }
        else if(argc == 4)
        {
            ucd_path = argv[2];
            assign_ucd_name = argv[3];
            ucd_util->upload_ucd(ucd_path, assign_ucd_name);
        }
        else
        {
            ucd_param_opt->print_command_info("upload");
            return -1;
        }
    }        
    else if(command_1 == "info")
    {
        if(argc == 3)
        {
            std::string ucd_path = argv[2];
            UCDataset * ucd = new UCDataset(ucd_path);
            ucd->print_ucd_info();
            delete ucd;
        }
        else
        {
            ucd_param_opt->print_command_info("info");
            return -1;
        }
    }
    else if(command_1 == "from_img")
    {
        if(argc == 4)
        {
            std::string img_path = argv[2];
            std::string ucd_name = argv[3];
            ucd_util->get_ucd_from_img_dir(img_path, ucd_name);
        }
        else
        {
            ucd_param_opt->print_command_info("from_img");
            return -1;
        }
    }
    else if(command_1 == "from_xml")
    {
        if(argc == 4)
        {
            std::string xml_dir = argv[2];
            std::string ucd_path = argv[3];
            ucd_util->get_ucd_from_xml_dir(xml_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info("from_xml");
            return -1;
        }
    }
    else if(command_1 == "from_json")
    {
        if(argc == 4)
        {
            std::string json_dir = argv[2];
            std::string ucd_path = argv[3];
            ucd_util->get_ucd_from_json_dir(json_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info("from_json");
            return -1;       
        }
    }
    else if(command_1 == "from_file")
    {
        if(argc == 4)
        {
            std::string file_dir = argv[2];
            std::string ucd_path = argv[3];
            ucd_util->get_ucd_from_file_dir(file_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
            return -1;       
        }
    }
    else if(command_1 == "parse_xml")
    {
        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string save_dir = argv[3];

            if(! is_file(ucd_path))
            {
                std::cout << "ucd_path not exists : " << ucd_path << std::endl;
                throw "ucd_path not exists";
            }
            ucd_util->parse_voc_xml(ucd_util->cache_img_dir, save_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info("parse_xml");
            return -1;
        }
    }
    else if(command_1 == "parse_json")
    {
        // todo 在 ucd_util 中实现一下，是一个图像一个图像进行保存，不要
        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string save_dir = argv[3];

            if(! is_file(ucd_path))
            {
                std::cout << "ucd_path not exists : " << ucd_path << std::endl;
                throw "ucd_path not exists";
            }
            ucd_util->parse_labelme_json(ucd_util->cache_img_dir, save_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info("parse_json");
        }
    }
    else if(command_1 == "show")
    {
        std::string uc;
        if(argc == 3)
        {
            uc = argv[2];
        }
        else if(argc == 2)
        {
            uc = "{UC}";
        }
        else
        {
            ucd_param_opt->print_command_info("show");
            return -1;
        }
        // 展示所有的下载路径，为了方便单张图片的信息下载查看，可以使用 curl 下载单个 uc 对应的信息
        std::cout << "load img      : http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/file/" + uc + ".jpg" << std::endl;
        std::cout << "load xml      : http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/file/" + uc + ".xml" << std::endl;
        std::cout << "load json     : http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/file/" + uc + ".json" << std::endl;
        std::cout << "load ucd      : http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/ucd/{ucd_name}.json" << std::endl;
        std::cout << "check         : http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/ucd/check" << std::endl;
    }
    else if(command_1 == "diff")
    {
        // todo 只是 uc 的差异，是否可以表示 tag 的差异 
        if(argc == 4)
        {
            std::string ucd_path_1 = argv[2];
            std::string ucd_path_2 = argv[3];
            ucd_util->ucd_diff(ucd_path_1, ucd_path_2);
        }
        else
        {
            ucd_param_opt->print_command_info("diff");
            return -1;
        }
    }
    else if(command_1 == "merge")
    {
        if(argc >= 5)
        {
            std::string save_path = argv[2];
            std::vector<std::string> ucd_path_vector;
            for(int i=0; i<argc-3; i++)
            {
                std::string each_ucd_path = argv[i+3];
                ucd_path_vector.push_back(each_ucd_path);
            }
            ucd_util->merge_ucds(save_path, ucd_path_vector);
        }
        else
        {
            ucd_param_opt->print_command_info("merge");
            return -1;
        }
    }
    else if(command_1 == "minus")
    {
        ucd_param_opt->not_ready("minus");
        return -1;

        if(argc == 5)
        {
            std::string ucd_save_path = argv[2];
            std::string ucd_path_1 = argv[3];
            std::string ucd_path_2 = argv[4];
            ucd_util->ucd_minus(ucd_save_path, ucd_path_1, ucd_path_2);
        }
        else
        {
            ucd_param_opt->print_command_info("minus");
            return -1;
        }
    }
    else if(command_1 == "meta")
    {
        if(argc == 2)
        {
            std::cout << "-----------------------------" << std::endl;
            std::cout << std::setw(15) << "ucd " << app_version << std::endl;
            std::cout << "-----------------------------" << std::endl;
            std::cout << "[server]" << std::endl;
            std::cout << "host          : " << host << std::endl;
            std::cout << "port          : " << port << std::endl;
            std::cout << "config_path   : " << config_path << std::endl;
            std::cout << "[sql]" << std::endl;
            std::cout << "sql_host      : " << sql_host << std::endl;
            std::cout << "sql_port      : " << sql_port << std::endl;
            std::cout << "sql_user      : " << sql_user << std::endl;
            std::cout << "sql_pwd       : " << sql_pwd << std::endl;
            std::cout << "sql_db        : " << sql_db << std::endl;
            std::cout << "[cache]" << std::endl;
            std::cout << "cache_dir     : " << cache_dir << std::endl;
            std::cout << "-----------------------------" << std::endl;
            return -1;
        }
        else if(argc == 3)
        {
            std::string attr_name = argv[2];
            if(attr_name == "host") {std::cout << host << std::endl ;}
            else if(attr_name == "port") {std::cout << port << std::endl ;}
            else if(attr_name == "config_path") {std::cout << config_path << std::endl ;}
            else if(attr_name == "sql_host") {std::cout << sql_host << std::endl ;}
            else if(attr_name == "sql_port") {std::cout << sql_port << std::endl ;}
            else if(attr_name == "sql_user") {std::cout << sql_user << std::endl ;}
            else if(attr_name == "sql_pwd") {std::cout << sql_pwd << std::endl ;}
            else if(attr_name == "sql_db") {std::cout << sql_db << std::endl ;}
            else if(attr_name == "cache_dir") {std::cout << cache_dir << std::endl ;}
            else{std::cout << "no attr name : " << attr_name << std::endl; }
            return -1;
        }
        else 
        {
            ucd_param_opt->print_command_info("meta");
            return -1;
        }
    }
    else if(command_1 == "--version" || command_1 == "-V")
    {
        std::cout << "uc_dataset : " << app_version << std::endl;
        return -1;
    }
    else if(command_1 == "set")
    {
        if(argc == 4)
        {
            xini_file_t xini_write(config_path);
            std::string option = argv[2]; 

            if(option == "host")
            {
                std::string host_new = argv[3];
                xini_write["server"]["host"] = host_new;
            }
            else if(option == "port")
            {
                int port_new = std::stoi(argv[3]);
                xini_write["server"]["port"] = port_new;      
            }
            else if(option == "sql_host")
            {
                std::string sql_host_new = argv[3];
                xini_write["sql"]["host"] = sql_host_new;
            }
            else if(option == "sql_port")
            {
                int sql_port_new = std::stoi(argv[3]);
                xini_write["sql"]["port"] = sql_port_new;   
            }
            else if(option == "sql_user")
            {
                std::string sql_user_new = argv[3];
                xini_write["sql"]["user"] = sql_user_new;
            }
            else if(option == "sql_pwd")
            {
                std::string sql_pwd_new = argv[3];
                xini_write["sql"]["pwd"] = sql_pwd_new;
            }
            else if(option == "sql_db")
            {
                std::string sql_db_name_new = argv[3];
                xini_write["sql"]["db"] = sql_db_name_new;
            }
            else if(option == "cache_dir")
            {
                std::string cache_dir_new = argv[3];
                // 
                if(!is_dir(cache_dir_new))
                {
                    std::cout << "cache dir not exist" << std::endl;
                    throw "cache dir not exist";
                }
                else
                {
                    // 创建对应的文件夹
                    std::string img_cache_dir = cache_dir_new + "/" + "img_cache";
                    if(! is_dir(img_cache_dir))
                    {
                        create_folder(img_cache_dir);
                    }
                }
                xini_write["cache"]["dir"] = cache_dir_new;
            }
            else
            {
                ucd_param_opt->print_command_info("set");
                return -1;
            }
            xini_write.dump(config_path);   
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info("set");
            return -1;
        }
    }
    else if(command_1 == "rename_img")
    {
        if(argc == 3)
        {
            std::string img_dir = argv[2];
            if(is_dir(img_dir))
            {
                SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
                sd_sql->rename_img_dir(img_dir);
                delete sd_sql;
            }
            else
            {
                std::cout << "image dir not exists : " << img_dir << std::endl;
                ucd_param_opt->print_command_info("rename_img");
                return -1;
            }
        }
        else
        {
            ucd_param_opt->print_command_info("rename_img");
            return -1;
        }
    }
    else if(command_1 == "rename_img_xml")
    {
        if(argc == 4)
        {
            std::string img_dir = argv[2];
            std::string xml_dir = argv[3];
            SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
            sd_sql->rename_img_xml_dir(img_dir, xml_dir);
            delete sd_sql;
        }
        else
        {
            ucd_param_opt->print_command_info("rename_img_xml");
            return -1;
        }
    }
    else if(command_1 == "rename_img_json")
    {
        if(argc == 4)
        {
            std::string img_dir = argv[2];
            std::string json_dir = argv[3];
            SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
            sd_sql->rename_img_json_dir(img_dir, json_dir);
            delete sd_sql;
        }
        else
        {
            ucd_param_opt->print_command_info("rename_img_json");
            return -1;
        }
    }
    else if(command_1 == "count_tags")
    {
        if(argc== 3)
        {
            std::string xml_dir = argv[2];
            if(ucd_util->is_ucd_path(xml_dir))
            {
                ucd_util->count_ucd_tags(xml_dir);
            }
            else if(is_dir(xml_dir))
            {
                count_tags(xml_dir);
            }
            else
            {
                std::cout << "ucd_path not exists or xml dir not exists : " << xml_dir << std::endl;
                ucd_param_opt->print_command_info("count_tags");
                return -1;
            }
        }
        else
        {
            ucd_param_opt->print_command_info("count_tags");
            return -1;
        }
    }
    else if(command_1 == "count_files")
    {
        if (argc== 4)
        {
            std::string xml_dir = argv[2];
            std::string recursive = argv[3];
            bool is_recursive = true;
            if((recursive == "false") || (recursive == "False") || (recursive == "0"))
            {
                is_recursive = false;
            }
            count_files(xml_dir, is_recursive);
        }
        else
        {
            ucd_param_opt->print_command_info("count_files");
            return -1;
        }
    }
    else if(command_1 == "cut_small_img")
    {
        if (argc == 5)
        {
            std::string ucd_path = argv[2];
            std::string save_dir = argv[3];
            std::string is_split_str = argv[4];
            bool is_split = true;
            if((is_split_str != "true") && (is_split_str != "True") && (is_split_str != "1"))
            {
                is_split = false;
            }
            ucd_util->cut_small_img(ucd_path, save_dir, is_split);
        }
        else
        {
            ucd_param_opt->print_command_info("cut_small_img");
            return -1;
        }
    }
    else if(command_1 == "crop_to_xml")
    {

        ucd_param_opt->not_ready(command_1);
        return -1;

        if (argc== 4)
        {
            std::string crop_dir = argv[2];
            std::string save_dir = argv[3];
            jotools::get_xml_from_crop_img(crop_dir, save_dir);
        }
        else
        {
            ucd_param_opt->print_command_info("crop_to_xml");
            return -1;
        }
    }
    else if(command_1 == "xml_check")
    {

        ucd_param_opt->not_ready(command_1);
        return -1;


        if ((argc == 6))
        {
            std::string xml_dir = argv[2];
            std::string img_dir = argv[3];
            int size_th = std::stoi(argv[4]);
            std::string remove_error_path = argv[5];
            
            bool remove_error = true;
            if((remove_error_path != "true") && (remove_error_path != "True") && (remove_error_path != "1"))
            {
                remove_error = false;
            }
            xml_check(xml_dir, img_dir, size_th, remove_error);
        }
        else
        {
            ucd_param_opt->print_command_info("xml_check");
            return -1;
        }
    }
    else if(command_1 == "say")
    {
        if((argc == 3) || (argc == 4) || (argc == 5))
        {
            std::string words = argv[2];
            int height, width;
            if(argc == 3)
            {
                height = 30;
                width = 30;
            }
            else if(argc == 4)
            {
                height = std::stoi(argv[3]);
                width = std::stoi(argv[3]);
            }
            else
            {
                height = std::stoi(argv[3]);
                width = std::stoi(argv[4]);
            }
            ucd_util->print_words(words, height, width);
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info("say");
        }
    }
    else if(command_1 == "filter")
    {
        ucd_param_opt->not_ready(command_1);
        return -1;

        // 对 ucd 进行过滤
        // used_label
        // conf
        // assign label list split by ,
        // filter by uc 是否为 uc 名字对数据进行划分，是否为 uc 直接与数据库上核对
        // 
    }
    else if(command_1 == "cache_info")
    {
        if(! is_dir(ucd_util->cache_img_dir))
        {
            std::cout << "cache dir is not exists" << std::endl;
            return -1;
        }

        int all_cache_img_count = 0;
        int exist_cache_img_count = 0;
        std::string cache_info, json_path;
        if((argc == 2) || (argc == 3))
        {
            if(argc == 3)
            {
                json_path = argv[2];
                if((! is_file(json_path)) || (json_path.substr(json_path.size()-5, json_path.size()) != ".json"))
                {
                    std::cout << "ucd path not exists : " << json_path << std::endl;
                    return -1;
                }

                UCDataset* ucd = new UCDataset(json_path);
                ucd->parse_ucd();
                std::string each_img_path;
                float exist_ratio;
                std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
                // 
                for(int i=0; i<ucd->uc_list.size(); i++)
                {
                    each_img_path = get_file_by_suffix_set(ucd_util->cache_img_dir, ucd->uc_list[i], suffix);
                    if(each_img_path != "")
                    {
                        exist_cache_img_count++;
                    }
                }
                exist_ratio = (float)exist_cache_img_count / (float)ucd->uc_list.size();
                std::cout << "----------------------------------" << std::endl;
                std::cout << "all img count         : " << ucd->uc_list.size() << std::endl;
                std::cout << "exist cache img count : " << exist_cache_img_count << std::endl;
                std::cout << "exist ratio           : " << exist_ratio * 100 << " %" << std::endl;
                std::cout << "----------------------------------" << std::endl;
                delete ucd;
            }
            else
            {
                // 没有参数，查看基础情况
                std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
                std::vector<std::string> img_path_vector;
                img_path_vector = get_all_file_path(ucd_util->cache_img_dir, suffix);
                all_cache_img_count = img_path_vector.size();
                std::cout << "----------------------------------" << std::endl;
                std::cout << "all cache img count   : " << all_cache_img_count << std::endl;
                std::cout << "----------------------------------" << std::endl;
            }
        }
        else
        {
            ucd_param_opt->print_command_info("cache_info");
        }
    }
    else if(command_1 == "cache_clear")
    {
        if(! is_dir(ucd_util->cache_img_dir))
        {
            std::cout << "cache dir is not exists" << std::endl;
            return -1;
        }

        if((argc == 2) || (argc == 3))
        {
            // 确认是否删除
            char confirm;
            std::cout << "Would clear cache:" << std::endl;
            std::cout << "Proceed (y/n)? ";
            std::cin >> confirm;
            if(confirm != 'y')
            {
                std::cout << "clear canceled" << std::endl;
                return -1;
            }
            // 删除 ucd 包含的所有图片
            if(argc == 3)
            {
                std::string ucd_path = argv[2];
                if(! ucd_util->is_ucd_path(ucd_path))
                {
                    std::cout << "ucd path not exists : " << ucd_path << std::endl;
                    throw "ucd path not exists";
                }
                else
                {
                    ucd_util->cache_clear(ucd_path);
                }
            }
            // 删除所有的文件
            else
            {
                ucd_util->cache_clear();
            }
        }
        else
        {
            ucd_param_opt->print_command_info("cache_clear");
        }
    }
    else if(command_1 == "cache_clean")
    {
        // 删除图片缓存中大小为 0 的空图片
        if(argc == 2)
        {
            ucd_util->cache_clean();
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
            return -1;
        }
    }
    else if(command_1 == "acc")
    {

        // 因为 没有记录 conf 信息，所以不好改变 config 得到不同 config 下面的结果

        // 生成结果为一个 ucd ，每个标签就是对应的检测结果

        // 为了推广使用和自己和别人的进行对比，举一些比较极端的例子，肯定会有差异的，指出这些差异的原因和解决方案，用 ucd 

        // 每个标签计算 acc res

        if(argc == 5)
        {
            std::string ucd_customer = argv[2];
            std::string ucd_standard = argv[3];
            std::string ucd_save_res = argv[4];
            jotools::DeteAcc* acc = new DeteAcc();
            acc->cal_acc_rec(ucd_customer, ucd_standard, ucd_save_res);
            delete acc;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "to_yolo")
    {
        if(argc == 4 || argc == 5)
        {
            std::string ucd_path = argv[2];
            std::string save_dir = argv[3];
            if(argc == 4)
            {
                ucd_util->parse_yolo_train_data(ucd_util->cache_img_dir, save_dir, ucd_path);
            }
            else
            {
                std::vector<std::string> label_list;
                std::string label_list_str = argv[4];
                label_list = pystring::split(label_list_str, ",");
                ucd_util->parse_yolo_train_data(ucd_util->cache_img_dir, save_dir, ucd_path, label_list);
            }
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "gif")
    {
        ucd_param_opt->not_ready(command_1);
        return -1;

        // 屏幕闪烁一些标准的 gif 图
        // 比如 你真棒，我很忙，你走开，别烦我
    }
    else if(command_1 == "attr")
    {
        // 修改 ucd 文件的属性信息，因为对于比较大的 ucd 打开自己去修改非常麻烦
        // ucd attr dataset_name test_name

        if(argc == 5)
        {
            std::string ucd_path = argv[2];
            if(ucd_util->is_ucd_path(ucd_path))
            {
                std::string attr_name = argv[3];
                std::string attr_value = argv[4];
                UCDataset * ucd = new UCDataset(ucd_path);
                ucd->change_attar(attr_name, attr_value);
                delete ucd;
            }
            else
            {
                std::cout << "ucd path not exists : " << ucd_path << std::endl;
                throw "ucd path not exists";
            }
        }
        else
        {
            ucd_param_opt->print_command_info("attr");
            return -1;
        }
    }
    else if(command_1 == "help")
    {
        if (argc == 3)
        {
            std::string command = argv[2];
            if(ucd_param_opt->has_command(command))
            {
                ucd_param_opt->print_command_info(command);
            }
            else
            {
                if(ucd_param_opt->has_simliar_command(command_1))
                {
                    ucd_param_opt->print_similar_command_info(command);
                }
            }
            return -1;
        }
        else if(argc ==2)
        {
            // 打印中文的注释
            ucd_param_opt->print_all_fun_chinese();
        }
        else
        {
            ucd_param_opt->print_command_info("help");
            return -1;
        }
    }
    else if(command_1 == "uc_check")
    {
        if(argc == 3)
        {
            std::string file_dir = argv[2];

            if(! is_dir(file_dir))
            {
                std::cout << "file_dir not exists : " << file_dir << std::endl;
                return -1;
            }

            std::vector<string> uc_vector;
            std::set<std::string> file_suffix {".jpg", ".JPG", ".png", ".PNG", ".json", ".xml"}; 
            std::vector<std::string> file_vector = get_all_file_path(file_dir, file_suffix);
        
            for(int i=0; i<file_vector.size(); i++)
            {
                std::string uc = get_file_name(file_vector[i]);
                uc_vector.push_back(uc);
            }

            SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
            std::map<std::string, bool> is_uc_map = sd_sql->check_uc_by_mysql(uc_vector);

            int is_uc = 0;
            int not_uc = 0;
            auto iter = is_uc_map.begin();
            while(iter != is_uc_map.end())
            {
                if(iter->second)
                {
                    is_uc += 1;
                }
                else
                {
                    not_uc += 1;
                }
                iter ++;
            }

            std::cout << "------------------------" << std::endl;
            std::cout << "is  uc count : " << is_uc << std::endl;
            std::cout << "not uc count : " << not_uc << std::endl;
            std::cout << "check name format for (.jpg, .JPG, .png, .PNG. .xml, .json) : " << std::endl;
            std::cout << "------------------------" << std::endl;
            delete sd_sql;
        }
        else
        {
            ucd_param_opt->print_command_info("uc_check");
            return -1;
        }        
    }
    else if(command_1 == "move_uc" || command_1 == "move_not_uc")
    {

        // the same name not different suffix 
        
        bool is_uc;
        if(command_1 == "move_uc")
        {
            is_uc = true;
        }
        else
        {
            is_uc = false;
        }

        if(argc == 4)
        {
            std::string file_dir = argv[2];
            std::string save_dir = argv[3];

            if(! is_dir(file_dir))
            {
                std::cout << "file_dir not exists : " << file_dir << std::endl;
                return -1;
            }

            std::vector<string> uc_vector;
            std::vector<std::string> file_vector = get_all_file_path(file_dir);

            for(int i=0; i<file_vector.size(); i++)
            {
                std::string uc = get_file_name(file_vector[i]);
                uc_vector.push_back(uc);
            }

            SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
            std::map<std::string, bool> is_uc_map = sd_sql->check_uc_by_mysql(uc_vector);
            delete sd_sql;

            std::vector<std::string> move_file_vector;
            for(int i=0; i<file_vector.size(); i++)
            {
                std::string file_name = get_file_name(file_vector[i]);
                if(is_uc_map[file_name] && is_uc)
                {
                    move_file_vector.push_back(file_vector[i]);
                }
                else if((! is_uc_map[file_name]) && (! is_uc))
                {
                    move_file_vector.push_back(file_vector[i]);
                }
            }
            move_file_vector_to_dir(move_file_vector, save_dir);
        }
        else
        {
            if(is_uc)
            {
                ucd_param_opt->print_command_info("move_uc");
            }
            else
            {
                ucd_param_opt->print_command_info("move_not_uc");
            }
            return -1;
        }   
    }
    else if(command_1 == "test")
    {

        UCDataset* ucd = new UCDataset("/home/ldq/ucd_dir/zgh.json");

        ucd->parse_ucd();

        for(int i=0; i<ucd->uc_list.size(); i++)
        {
        
            ucd_util->load_img_with_assign_uc("/home/ldq/ucd_dir/del_img", ucd->uc_list[i]);

        }

        return 0;

    }
    else if(command_1 == "filter_by_tags")
    {
        // 根据标签进行过滤
        ucd_param_opt->not_ready(command_1);
        return -1;

    }
    else if(command_1 == "filter_by_nms")
    {
        if(argc == 6)
        {
            std::string ucd_path = argv[2];
            std::string ucd_save_path = argv[3];
            float nms_th = std::stof(argv[4]);
            std::string ignore_tag_str = argv[5];
            bool ignore_tag = true;

            if((ignore_tag_str != "true") && (ignore_tag_str != "True") && (ignore_tag_str != "1"))
            {
                ignore_tag = false;
            }

            //
            if(! is_file(ucd_path))
            {
                std::cout << "ucd_path not exists : " << ucd_path;
                throw "ucd_path not exists";
            }

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->filter_by_nms(nms_th, ignore_tag);
            ucd->save_to_ucd(ucd_save_path);
            delete ucd; 
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "filter_by_conf")
    {
        if(argc == 5)
        {
            std::string ucd_path = argv[2];
            std::string ucd_save_path = argv[3];
            float conf_th = std::stof(argv[4]);

            //
            if(! is_file(ucd_path))
            {
                std::cout << "ucd_path not exists : " << ucd_path;
                throw "ucd_path not exists";
            }

            std::cout << ucd_path << std::endl;

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->filter_by_conf(conf_th);
            ucd->save_to_ucd(ucd_save_path);
            delete ucd; 
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "img_url")
    {

        ucd_param_opt->not_ready(command_1);
        return -1;

        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string save_txt_path = argv[3];
            
            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd();

            for(int i=0; i<ucd->uc_list.size(); i++)
            {
                std::cout << "http://" + ucd_util->host + ":" + std::to_string(ucd_util->port) + "/file/" + ucd->uc_list[i] + ".jpg" << std::endl;
            }

            // todo 保存为  txt
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "uc_analysis")
    {
        if(argc == 3)
        {
            std::string ucd_path = argv[2];
            ucd_util->uc_analysis(ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "conf_analysis")
    {
        // ucd 的置信度分布
        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            int seg_count = std::stoi(argv[3]);
            ucd_util->conf_analysis(ucd_path, seg_count);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "area_analysis")
    {
        ucd_param_opt->not_ready(command_1);
        return -1;

        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            int seg_count = std::stoi(argv[3]);
            ucd_util->area_analysis(ucd_path, seg_count);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "shape_analysis")
    {
        // 对图像的大小进行分析和聚类，图像的大小有一定是有其对应的规律的
        
        // 输出的时候只选取其中的最多的集中的长宽进行输出
        

    }
    else if(command_1 == "server_info")
    {
        // check if sshpass is installed, apt list --installed | grep sshpass/focal
        // c++ run bash 
        // use sshpass -p txkj ssh txkj@192.168.3.101 "nvidia-smi"
        // ucd server_info 221 101 
        // drive | gpu type | free space (gpu)

        // std::system("sshpass -p txkj ssh txkj@192.168.3.101 nvidia-smi"); // 执行 UNIX 命令 "ls -l >test.txt"
        // std::cout << "200 : ";
        
        std::system("echo -n '221 : '  && nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        std::system("echo -n '200 : '  && sshpass -p txkj2020 ssh txkj@192.168.3.200 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        std::system("echo -n '155 : '  && sshpass -p ldq ssh ldq@192.168.3.155 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        std::system("echo -n '21  : '  && sshpass -p ldq ssh ldq@192.168.3.21 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        std::system("echo -n '101 : '  && sshpass -p txkj ssh txkj@192.168.3.101 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        std::system("echo -n '132 : '  && sshpass -p txkj ssh txkj@192.168.3.101 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        
        // std::system("sshpass -p txkj2020 ssh txkj@192.168.3.200 nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}'"); // 执行 UNIX 命令 "ls -l >test.txt"
        
        
        // std::cout << std::ifstream("test.txt").rdbuf();
        // std::system("sudo su ; ucd | grep filter"); // 执行 UNIX 命令 "ls -l >test.txt"
        // std::system("mkdir /home/jokker/test");


        // nvidia-smi | sed -n 3p | awk '{print $4, $5, $6}', 查看驱动版本
        // 


        return -1;

    }
    else if(command_1 == "get")
    {
        if(argc == 4)
        {
            std::string attr_name = argv[2];
            std::string ucd_path = argv[3];

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd();
            if(attr_name == "dataset_name")
            {
                std::cout << ucd->dataset_name << std::endl;
            }
            else if(attr_name == "model_name")
            {
                std::cout << ucd->model_name << std::endl;
            }
            else if(attr_name == "model_version")
            {
                std::cout << ucd->model_version << std::endl;     
            }
            else if(attr_name == "add_time")
            {
                std::cout << ucd->add_time << std::endl;     
            }
            else if(attr_name == "update_time")
            {
                std::cout << ucd->update_time << std::endl;     
            }
            else if(attr_name == "describe")
            {
                std::cout << ucd->describe << std::endl;     
            }
            else if(attr_name == "label_used")
            {
                for(int i=0; i<ucd->label_used.size()-1; i++)
                {
                    std::cout << ucd->label_used[i] << ",";
                }
                std::cout << ucd->label_used[ucd->label_used.size()-1] << std::endl;
            }
            else if(attr_name == "uc_list")
            {
                for(int i=0; i<ucd->uc_list.size()-1; i++)
                {
                    std::cout << ucd->uc_list[i] << ",";
                }
                std::cout << ucd->uc_list[ucd->uc_list.size()-1] << std::endl;
            }
            else if(attr_name == "uc_count")
            {
                std::cout << ucd->uc_list.size() << std::endl;     
            }
            else if(attr_name == "label_used_count")
            {
                std::cout << ucd->label_used.size() << std::endl;     
            }
            else
            {
                std::cout << "attr_name not in [dataset_name, uc_count, label_used_count, model_name, model_version, add_time, update_time, describe, label_used, uc_list]" << std::endl;     
            }
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);  
        }
    }
    else if(command_1 == "has_uc")
    {
        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string uc = argv[3];

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd();

            for(int i=0; i<ucd->uc_list.size(); i++)
            {
                if(ucd->uc_list[i] == uc)
                {
                    std::cout << "True" << std::endl;
                    return -1;
                }
            }
            std::cout << "False" << std::endl;
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "drop")
    {
        if(argc == 5)
        {
            std::string attr_name = argv[2];
            std::string ucd_path = argv[3];
            std::string save_ucd_path = argv[4];

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd();
            if(attr_name == "dataset_name")
            {
                ucd->dataset_name = "";
            }
            else if(attr_name == "model_name")
            {
                ucd->model_name = "";
            }
            else if(attr_name == "model_version")
            {
                ucd->model_version = "";    
            }
            else if(attr_name == "add_time")
            {
                ucd->add_time = -1;  
            }
            else if(attr_name == "update_time")
            {
                ucd->update_time = -1;
            }
            else if(attr_name == "describe")
            {
                ucd->describe = ""; 
            }
            else if(attr_name == "label_used")
            {
                ucd->label_used = {};
            }
            else if(attr_name == "uc_list")
            {
                ucd->uc_list = {};
            }
            else if(attr_name == "object_info")
            {
                ucd->object_info = {};
            }
            else
            {
                std::cout << "attr_name not in [dataset_name, object_info, model_name, model_version, add_time, update_time, describe, label_used, uc_list]" << std::endl;     
            }
            ucd->save_to_ucd(save_ucd_path);
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);  
        }
    }
    else if(command_1 == "uc_info")
    {
        // 打印指定的 uc 的信息

        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string uc = argv[3];
            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);

            // size_info
            std::cout << "-----------------------" << std::endl;
            if(ucd->size_info.count(uc) > 0)
            {
                std::cout << "(w, h) : (" << ucd->size_info[uc][0] << ", " << ucd->size_info[uc][1] << ")" << std::endl;
            }
            std::cout << "-----------------------" << std::endl;

            // obj_info
            if(ucd->object_info.count(uc) > 0)
            {
                for(int i=0; i<ucd->object_info[uc].size(); i++)
                {
                    ucd->object_info[uc][i]->print_info();
                    std::cout << "-----------------------" << std::endl;
                }
            }
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "sub")
    {
        if(argc == 6)
        {
            std::string ucd_path = argv[2];
            std::string save_path = argv[3];
            int count = std::stoi(argv[4]);
            std::string is_random_str = argv[5];
            bool is_random = true;
            if((is_random_str == "false") || (is_random_str == "False") || (is_random_str == "0"))
            {
                is_random = false;
            }
            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->get_sub_ucd(count, is_random, save_path);
            return -1;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "update_uc_by_obj")
    {
        // 去除没有 obj_info 信息的 uc
        if(argc == 4)
        {
            std::string ucd_path = argv[2];
            std::string save_path = argv[3];

            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->update_uc_list_by_object_info(save_path);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "update")
    {
        // 将最新的下载包放在 80 服务器上，下载到本地，放到对应的目录下即可
        std::string app_dir = "/home/ldq/Apps_jokker";
        if(! is_dir(app_dir))
        {
            create_folder(app_dir);
        }

        if(! is_dir(app_dir))
        {
            std::cout << "create app_dir failed : " << app_dir << std::endl;
            return -1;
        }
        
        if((argc == 3) || (argc == 2))
        {
            std::string version;
            std::string save_path = "/home/ldq/Apps_jokker/ucd_app_temp.zip";
            
            if(argc == 3)
            {
                version = argv[2];
            }
            else
            {
                version = "latest";
            }
            ucd_util->load_ucd_app(version, save_path);

            if(! is_file(save_path))
            {
                std::cout << "can't load file : " << save_path << std::endl;
                return -1;
            }

            // 解压缩
            std::system("unzip -n /home/ldq/Apps_jokker/ucd_app_temp.zip -d /home/ldq/Apps_jokker/ucd_app_temp");
            std::system("cp -r /home/ldq/Apps_jokker/ucd_app_temp/* /home/ldq/Apps_jokker");
            std::system("cp -r /home/ldq/Apps_jokker/so_dir/* /usr/lib");
            std::system("rm -r /home/ldq/Apps_jokker/ucd_app_temp");
            sleep(0.5);
            std::system("rm -r /home/ldq/Apps_jokker/ucd_app_temp.zip");
            sleep(0.5);
            std::system("chmod 777 /home/ldq/Apps_jokker/* -R");
            std::cout << "--------------------------------------" << std::endl;
            std::cout << "change ucd version by :" << std::endl;
            std::cout << "  vim ~/.bash_aliases" << std::endl;
            std::cout << "  source ~/.bash_aliases" << std::endl;
            std::cout << "" << std::endl;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "split")
    {
        // 随机将数据集按照一定的比例分为两个部分
        if(argc == 6)
        {
            std::string ucd_path = argv[2];
            std::string ucd_part_a = argv[3];
            std::string ucd_part_b = argv[4];
            float ratio = std::stof(argv[5]);
            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->split(ucd_part_a, ucd_part_b, ratio);
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
        }
    }
    else if(command_1 == "absorb")
    {
        // 从其他 ucd 中吸收需要的内容 size_info, object_info
        if(argc == 6)
        {
            std::string ucd_path = argv[2];
            std::string meat_ucd_path = argv[3];
            std::string save_path = argv[4];
            std::string attr = argv[5];
            // 
            UCDataset* ucd = new UCDataset(ucd_path);
            ucd->parse_ucd(true);
            ucd->absorb(meat_ucd_path, save_path, attr);
            delete ucd;
        }
        else
        {
            ucd_param_opt->print_command_info(command_1);
            return -1;
        }
    }
    else if(command_1 == "buddha_bless")
    {
        std::cout << "               _ooOoo_                " << std::endl;
        std::cout << "              o8888888o               " << std::endl;
        std::cout << "              88' . '88               " << std::endl;
        std::cout << "              (| -_- |)               " << std::endl;
        std::cout << "              O\  =  /O               " << std::endl;
        std::cout << "           ____/`---'\____            " << std::endl;
        std::cout << "         .'  \|       |    `.         " << std::endl;
        std::cout << "        /  \|||||  :  |||   |        " << std::endl;
        std::cout << "       /  _||||| -:- |||||- |        " << std::endl;
        std::cout << "       |   | \  -  / |   |            " << std::endl;
        std::cout << "       | \_|  ''\---/''  |   |        " << std::endl;
        std::cout << "       \  .-\__  `-`  ___/-. /        " << std::endl;
        std::cout << "     ___`. .'  /--.--\  `. . __       " << std::endl;
        std::cout << "  ."" '<  `.___\_<|>_/___.'  >'"".    " << std::endl;
        std::cout << " | | :  `- \`.;`\ _ /`;.`/ - ` : | |  " << std::endl;
        std::cout << " \  \ `-.   \_ __\ /__ _/   .-` /  /  " << std::endl;
        std::cout << "===`-.____`-.___\_____/___.-`____.-'==" << std::endl;
        std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
        std::cout << "        Buddha Bless, No Bug !        " << std::endl;
    }
    else if(ucd_param_opt->has_simliar_command(command_1))
    {
        ucd_param_opt->print_similar_command_info(command_1);
        return -1;
    }
    else
    {
        ucd_param_opt->print_all_fun();
        return -1;
    }
    
    delete ucd_util;
    delete ucd_param_opt;

	return 1;
}
