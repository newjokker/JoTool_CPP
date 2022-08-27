
#include <iostream>
#include <fstream>
#include <set>
#include <pwd.h>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include "include/crow_all.h"
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
using json = nlohmann::json;
#include "include/lablelmeObj.hpp"


using namespace jotools;
using namespace std;


// nginx 负载均衡，可以在风火轮上部署，转到 111 和 209 服务器上

// 完善路径拼接（c++多一些 // 不会造成路径错误，少一些就会报错）

// 完成 C++ 版本的 文件 服务，再部署到 docker 上面，这样在哪个服务器上都能方便进行启动

// 整合了 xml 信息之后，连平时的测试结果都可以进行存储，这样方便后期的获取对比

// 获取数据是按照顺序还是随机获取需要修改配置文件 ucdconfig.ini， 

// 完善报错信息，对于 upload 失败等进行提示

// merge 支持 xml 合并（如何快速去重是个难以解决的问题，修改保存的结构，vector 改为 set）

// 积极发挥服务化的各项功能，比如提交合并的数据

// todo json 读取大文件太慢，能不能只读取需要的部分，比如打印 info 信息，不需要后面的 xml_info 信息，是否可以不读取到内存里，不行的话就直接自己写一个解析 json 的代码，用做快速解析

// 一个专门的测试文件，针对每一个函数写一个对应的测试函数，就像在 guihub 上看到别人写的那样

// 发布功能，快速部署到所有机器上

// 下载某一些图片报错，可能是没有按照各个后缀依次寻找图片

// 快速测试代码的稳定性，重要的是同步功能，（保留下载到本地的选项）

// 交互设计之类的全部抄 git 的

// 参数的有效性检查都是在函数里面去做，在外面不需要去做

// 解释一下斜框矩形为什么不被包括在 labelme 中，因为不是标准结构，labelme 一直使用的点对结构信息进行存储

// 斜框先不进行处理，先去掉

// 没有地方存储置信度的，这个要注意一点

// 忘记 xml，img，json 只记得 ucd 即可

// 调整所有操作的粒度，以 uc 和 batch_uc 为单位，

// 查看是不是一个文件夹中的数据都是 uc 命名 

// 不是 uc 名字的文件处理 移动等

// todo merge 合并时候连带着 object_info 一起合并


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

    // time
    clock_t start_time, end_time;
    start_time = clock();    

    // server
    std::string host = "192.168.3.111";
    int port = 11101;
    std::string config_path;
    
    // sql info 
    int sql_port = 3306;
    std::string sql_host = "192.168.3.101";
    std::string sql_user = "root";
    std::string sql_pwd = "root123";
    std::string sql_db = "Saturn_Database_beta";
    
    // version
    std::string app_version = "v1.2";

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
    
    if(! is_dir(cache_dir))
    {
        std::cout << "cache_dir not exists, edit ucdconfig.ini cache/cache_dir : " << std::endl;
        std::cout << "edit ucdconfig.ini with ucd set " << std::endl;
        throw "cache_dir not exists!";
    }

    UCDatasetUtil* ucd_util = new UCDatasetUtil(host , port, cache_dir);
    std::string command_1 = argv[1];

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
            //
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
            std::string save_img_dir = save_dir + "/" + "img";
            std::string save_xml_dir = save_dir + "/" + "xml";
        
            if(save_mode[0] == '0')
            {
                need_img = false;
            }
            else
            {
                need_img = true;
                if(! is_dir(save_img_dir))
                {
                    create_folder(save_img_dir);
                }
            }

            if(save_mode[1] == '0')
            {
                need_xml = false;
            }
            else
            {
                need_xml = true;
                if(! is_dir(save_xml_dir))
                {
                    create_folder(save_xml_dir);
                }
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

            if(need_img){ucd_util->load_img(save_img_dir, uc_vector);}
            if(need_xml){ucd_util->load_xml(save_xml_dir, uc_vector);}
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
            UCDataset * ucd_info = new UCDataset(ucd_path);
            ucd_info->print_ucd_info();
            delete ucd_info;
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
            std::string xml_dir = argv[2];
            std::string ucd_path = argv[3];
            ucd_util->get_ucd_from_json_dir(xml_dir, ucd_path);
        }
        else
        {
            ucd_param_opt->print_command_info("from_json");
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
        if(argc = 3)
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

        // ucdataset 增加 merge 函数，用于两两合并
        // 合并 uc 和 obj

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
        if (argc == 6)
        {
            // get parameter
            std::string xml_dir = argv[2];
            std::string img_dir = argv[3];
            std::string save_dir = argv[4];
            std::string is_split_str = argv[5];
            bool is_split = true;
            if((is_split_str != "true") && (is_split_str != "True") && (is_split_str != "1"))
            {
                is_split = false;
            }
            // cut_small_img(xml_dir, img_dir, save_dir, is_split);
            ucd_util->cut_small_img(img_dir, xml_dir, save_dir, is_split);
        }
        else
        {
            ucd_param_opt->print_command_info("cut_small_img");
            return -1;
        }
    }
    else if(command_1 == "crop_to_xml")
    {
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
    else if(command_1 == "format_xml")
    {
        // 传入 img 的话就规范 height, width
        // 格式化没有 prob 字段，读取 xml报错 的问题
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

        int all_cache_img_count, exist_cache_img_count;
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
    else if(command_1 == "acc")
    {
        // 对比两个 ucd 的对比结果，
        // ucd 作为标准结果，只允许 ucd 之间进行对比
        // 可以设置是否保存标准的画图
        // 直接画出 0.1 ~ 0.9 的统计数据
    }
    else if(command_1 == "to_yolo")
    {
        // 转为 yolo 的训练格式，vit 的训练格式 faster rcnn csra 的训练格式
    }
    else if(command_1 == "gif")
    {
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
        // 输出指定 command 详细的参考
        if (argc == 3)
        {
            //
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
        else
        {
            ucd_param_opt->print_command_info("help");
            return -1;
        }
    }
    else if(ucd_param_opt->has_simliar_command(command_1))
    {
        ucd_param_opt->print_similar_command_info(command_1);
        return -1;
    }
    else if(command_1 == "uc_check")
    {
        // 查看是不是所有的文件都是 uc 格式命名的，去数据库中进行对比
        // 对于不符合 uc 规范的文件可以进行路径提取，或者文件移动到指定文件夹
    }
    else
    {
        ucd_param_opt->print_all_fun();
        return -1;
    }

    delete ucd_util;
    delete ucd_param_opt;                         
    end_time = clock();
    std::cout << "---------------" << std::endl;
    std::cout << "use time " << (double)(end_time-start_time)/CLOCKS_PER_SEC << " s" << std::endl;

	return 1;
}
