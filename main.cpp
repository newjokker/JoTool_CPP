
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

using namespace jotools;
using namespace std;


// nginx 负载均衡，可以在风火轮上部署，转到 111 和 209 服务器上

// 完善路径拼接（c++多一些 // 不会造成路径错误，少一些就会报错）

// 完成 C++ 版本的 文件 服务，再部署到 docker 上面，这样在哪个服务器上都能方便进行启动

// 生成  path_list 方便 v2 版本的测试，一个 txt 里面写满了 读取的 path

// jo_post 实现 v0 ~ v4 的测试
// 

// 将更多的内容服务化，写一个轻量化的服务，这样最大化利用缓存的数据，可以搞成 docker 服务

// 将 xml 信息整合到 ucd 中，实现最轻量化的 xml 数据（只支持 正框的数据） ucd from_img, ucd_from_img_xml(保存 xml 信息)

// 整合了 xml 信息之后，连平时的测试结果都可以进行存储，这样方便后期的获取对比

// 获取数据是按照顺序还是随机获取需要修改配置文件 ucdconfig.ini， 

// 完善报错信息，对于 upload 失败等进行提示

// merge 之类的信息都需要支持 合并 xml 信息

// 积极发挥服务化的各项功能，比如提交合并的数据

// xml 读取的时候


void print_info(std::string command="*")
{

    if(command=="save" || command =="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "下载 图片|标注|元信息, ucd save json_path save_dir save_mode(image,xml,json) {need_count}" << std::endl;
    }
    if(command=="parse" || command =="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "下载 图片 可以选择从 json 解析出 xml, ucd parse json_path save_dir save_mode(image,xml_from_json) {need_count}" << std::endl;
    }
    if(command=="check" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "查看所有在线数据集, ucd check" << std::endl;
    }
    if(command=="show" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "查看所有下载路径, ucd show {uc}" << std::endl;
    }
    if(command=="delete" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "删除在线数据集,无法删除官方数据集 ucd delete ucd_name" << std::endl;
    }
    if(command=="load" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "下载在线数据集 ucd load ucd_name save_path|save_dir " << std::endl;
    }
    if(command=="upload" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "上传数据集到网络 ucd upload ucd_path {ucd_name}" << std::endl;
    }
    if(command=="from" || command=="from_img" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "本地文件生成数据集 ucd from img_dir ucd_save_path" << std::endl;
    }
    if(command=="from_img_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "本地文件生成数据集 ucd from_img_xml img_dir xml_dir ucd_save_path" << std::endl;
    }
    if(command=="info" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "查看数据集信息 ucd info ucd_path" << std::endl;
    }
    if(command=="meta" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "查看配置信息 ucd meta" << std::endl;
    }
    if(command=="set" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "设置配置信息 ucd set host | port value" << std::endl;
    }
    if(command=="merge" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "合并数据集 ucd merge save_path ucd_path1 ucd_path2 ..." << std::endl;
    }
    if(command=="minus" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "减操作数据集 ucd minus save_path ucd_path1 ucd_path2" << std::endl;
    }
    if(command=="diff" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "比较数据集 ucd diff ucd_path1 ucd_path2" << std::endl;
    }
    // 不属于 ucd 后期进行删除
    if(command=="rename_img" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "重命名数据集 ucd rename_img img_dir" << std::endl;
    }
    if(command=="rename_img_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "重命名数据集 ucd rename_img_xml img_dir xml_dir" << std::endl;
    }
    if(command=="count_tags" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "统计xml 标签 ucd count_tags xml_dir" << std::endl;
    }
    if(command=="count_files" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "统计文件夹中各后缀的文件数 ucd count_files file_dir recursive(true|1|True|false|0|False)" << std::endl;
    }
    if(command=="cut_small_img" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "统计xml 标签 ucd cut_small_img img_dir xml_dir save_dir is_split(true|1|True|false|0|False)" << std::endl;   
    }
    if(command=="crop_to_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "截图生成xml ucd crop_to_xml crop_dir, save_dir" << std::endl;
    }
    if(command=="check_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "检查xml是否符合标准 ucd check_xml xml_dir img_dir size_th remove_error(true|1|True|false|0|False)" << std::endl;
    }
    if(command=="format_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "将 xml 进行标准化 ucd format_xml xml_dir {img_dir}" << std::endl;
    }
    std::cout << "-------------------------------------------------------" << std::endl;
}


int main(int argc, char ** argv)
{
    if ((argc < 2))
    {
        std::cout << "need parameter number >= 1 get : " << argc-1 << std::endl;
        print_info("*");
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
    std::string app_version = "v1.1";

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

    UCDatasetUtil* ucd_util = new UCDatasetUtil(host , port, cache_dir);
    std::string command_1 = argv[1];

    if(command_1 == "check")
    {
        if(argc == 2)
        {
            ucd_util->check_ucd();
        }
        else
        {
            print_info("check");
            return -1;
        }
    }
    else if(command_1 == "load")
    {        
        if(argc < 2)
        {
            print_info("load");
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
            print_info("load");
            return -1;
        }
        ucd_util->save_ucd(ucd_name, ucd_save_path);
    }
    else if(command_1 == "delete")
    {
        if(argc != 3)
        {
            print_info("delete");
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
        if((argc != 5) && (argc != 6))
        {
            print_info("save");
            return -1;
        }
        else
        {
            std::string json_path = argv[2];
            std::string save_dir = argv[3];
            std::string save_mode = argv[4];
            // need assign number of data
            int need_count = -1;
            if(argc == 6)
            {
                need_count = std::stoi(argv[5]);
            }

            // json_path
            if(! is_file(json_path))
            {
                std::cout << "json_path not exists : " << json_path << std::endl;
                throw "json_path not exists";
            }
            // save_path
           if(! is_dir(save_dir))
            {
                std::cout << "save_dir not exists : " << save_dir << std::endl;
                throw "save_dir not exists";
             }
            // save_mode
            if(save_mode.size() != 2)
            {
                std::cout << "save_mode illeagal, need save_mode such as 11 | 10  " << save_dir << std::endl;
                throw "save_mode illeagal";
            }
            bool need_img, need_xml, need_json;
            
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
            ucd_util->json_path = json_path;
            ucd_util->save_img_xml_json(save_dir, need_img, need_xml, need_count);
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
            print_info("upload");
            return -1;
        }
    }        
    else if(command_1 == "info")
    {
        if(argc != 3)
        {
            print_info("info");
            return -1;
        }

        std::string json_path = argv[2];
        UCDataset * ucd_info = new UCDataset(json_path);
        ucd_info->parse_json_info();
        ucd_info->print_json_info();
        delete ucd_info;
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
            print_info("from");
            return -1;
        }
    }
    else if(command_1 == "from_img_xml")
    {
        // 保存 xml 信息到 ucd 中去
        if(argc == 5)
        {
            std::string img_dir = argv[2];
            std::string xml_dir = argv[3];
            std::string ucd_name = argv[4];
            ucd_util->get_ucd_from_img_xml_dir(img_dir, xml_dir, ucd_name);
        }
        else
        {
            print_info("from_img_xml");
            return -1;
        }
    }
    else if(command_1 == "parse")
    {
        // ucd 中解压出 xml 信息，同时可以解析出 img 信息
        if((argc == 5) || (argc == 6))
        {
            std::string json_path = argv[2];
            std::string save_dir = argv[3];
            std::string save_mode = argv[4];
            // need assign number of data
            int need_count = -1;
            if(argc == 6)
            {
                need_count = std::stoi(argv[5]);
            }

            // json_path
            if(! is_file(json_path))
            {
                std::cout << "json_path not exists : " << json_path << std::endl;
                throw "json_path not exists";
            }
            // save_path
           if(! is_dir(save_dir))
            {
                std::cout << "save_dir not exists : " << save_dir << std::endl;
                throw "save_dir not exists";
             }
            // save_mode
            if(save_mode.size() != 2)
            {
                std::cout << "save_mode illeagal, need save_mode such as 11 | 10  " << save_dir << std::endl;
                throw "save_mode illeagal";
            }
            bool need_img, need_xml, need_json;
            
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
            ucd_util->json_path = json_path;
            ucd_util->save_img_xml_json(save_dir, need_img, false, need_count);
            // parse xml from ucd 
            ucd_util->save_xml(save_dir, need_count);
        }
        else
        {
            print_info("parse");
            return -1;
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
            print_info("show");
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
        if(argc == 4)
        {
            std::string ucd_path_1 = argv[2];
            std::string ucd_path_2 = argv[3];
            ucd_util->ucd_diff(ucd_path_1, ucd_path_2);
        }
        else
        {
            print_info("diff");
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
            print_info("merge");
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
            print_info("minus");
            return -1;
        }
    }
    else if(command_1 == "meta")
    {
        if(argc == 2)
        {
            std::cout << "-----------------------------" << std::endl;
            std::cout << "[server]" << std::endl;
            std::cout << "host          : " << host << std::endl;
            std::cout << "port          : " << port << std::endl;
            std::cout << "config path   : " << config_path << std::endl;
            std::cout << "[sql]" << std::endl;
            std::cout << "sql_host      : " << sql_host << std::endl;
            std::cout << "sql_port      : " << sql_port << std::endl;
            std::cout << "sql_user      : " << sql_user << std::endl;
            std::cout << "sql_pwd       : " << sql_pwd << std::endl;
            std::cout << "sql_db        : " << sql_db << std::endl;
            std::cout << "[cache]" << std::endl;
            std::cout << "cache_dir     : " << cache_dir << std::endl;
            std::cout << "[version]" << std::endl;
            std::cout << "uc_dataset    : " << app_version << std::endl;
            std::cout << "-----------------------------" << std::endl;
            return -1;
        }
        else
        {
            print_info("meta");
            return -1;
        }
    }
    else if(command_1=="--version" || command_1=="-V")
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
                print_info("set");
                return -1;
            }
            xini_write.dump(config_path);   
            return -1;
        }
        else
        {
            print_info("set");
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
                print_info("rename_img");
                return -1;
            }
        }
        else
        {
            print_info("rename_img");
            return -1;
        }
    }
    else if(command_1 == "rename_img_xml")
    {
        if(argc == 4)
        {
            std::string img_dir = argv[2];
            std::string xml_dir = argv[3];
            if(is_dir(img_dir) && is_dir(xml_dir))
            {
                SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);
                sd_sql->rename_img_xml_dir(img_dir, xml_dir);
                delete sd_sql;
            }
            else
            {
                std::cout << "image dir or xml dir not exists : " << img_dir << std::endl;
                print_info("rename_img_xml");
                return -1;
            }
        }
        else
        {
            print_info("rename_img_xml");
            return -1;
        }
    }
    else if(command_1 == "count_tags")
    {
        if (argc== 3)
        {
            std::string xml_dir = argv[2];
            count_tags(xml_dir);
        }
        else
        {
            print_info("count_tags");
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
            print_info("count_files");
            return -1;
        }
    }
    else if(command_1 == "cut_small_img")
    {
        if (argc== 6){
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
            cut_small_img(xml_dir, img_dir, save_dir, is_split);
        }
        else
        {
            print_info("cut_small_img");
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
            print_info("crop_to_xml");
            return -1;
        }
    }
    else if(command_1 == "check_xml")
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
            print_info("check_xml");
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
        //大屏幕打印
    }
    else if(command_1 == "filter")
    {
        // 对 ucd 进行过滤
        // used_label
        // conf
        // assign label list split by ,
    }
    else if(command_1 == "cache")
    {
        // 缓存信息的获取（有多少张缓存，指定的 json 有多少在缓存中，缓存率百分之多少）
        // 缓存的清空
        // 缓存信息的设置，最大缓存量。等

        if(! is_dir(ucd_util->cache_img_dir))
        {
            std::cout << "cache dir is not exists" << std::endl;
            return -1;
        }

        int all_cache_img_count, exist_cache_img_count;
        std::string cache_info, json_path;
        if((argc == 3) || (argc == 4))
        {
            cache_info = argv[2];
            if(cache_info == "info")
            {
                if(argc == 4)
                {
                    json_path = argv[3];
                    if((! is_file(json_path)) || (json_path.substr(json_path.size()-5, json_path.size()) != ".json"))
                    {
                        std::cout << "ucd path not exists : " << json_path << std::endl;
                        print_info("cache");
                        return -1;
                    }

                    UCDataset* ucd = new UCDataset(json_path);
                    ucd->parse_json_info();
                    std::string each_img_path;
                    float exist_ratio;
                    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};

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
        }
    }
    else if(command_1 == "acc")
    {
        // 对比两个 ucd 的对比结果，
        // 可以设置是否保存标准的画图
        // 直接画出 0.1 ~ 0.9 的统计数据
    }
    else if(command_1 == "to")
    {
        // 转为 yolo 的训练格式，vit 的训练格式 faster rcnn csra 的训练格式
    }
    else if(command_1 == "gif")
    {
        // 屏幕闪烁一些标准的 gif 图
        // 比如 你真棒，我很忙，你走开，别烦我
    }
    else
    {
        print_info("*");
        return -1;
    }

    delete ucd_util;
    end_time = clock();
    std::cout << "---------------" << std::endl;
    std::cout << "use time " << (double)(end_time-start_time)/CLOCKS_PER_SEC << " s" << std::endl;

	return 1;
}
