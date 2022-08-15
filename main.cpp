
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



void print_info(std::string command="*")
{

    if(command=="save" || command =="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "下载 图片|标注|元信息, ucd save json_path save_dir save_mode(image,xml,json) {need_count}" << std::endl;
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
    if(command=="from" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "本地文件生成数据集 ucd from img_dir ucd_save_path" << std::endl;
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
        std::cout << "减操作数据集 ucd minus save_path ucd_path1 ucd_path2 ..." << std::endl;
    }
    if(command=="diff" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "比较数据集 ucd diff ucd_path1 ucd_path2 ..." << std::endl;
    }
    if(command=="rename_img" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "重命名数据集 ucd rename_img img_dir ..." << std::endl;
    }
    if(command=="rename_img_xml" || command=="*")
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "重命名数据集 ucd rename_img_xml img_dir xml_dir ..." << std::endl;
    }
    std::cout << "-------------------------------------------------------" << std::endl;
    throw "error";
}


int main(int argc, char ** argv)
{
    if ((argc < 2))
    {
        std::cout << "need parameter number >= 1 get : " << argc-1 << std::endl;
        print_info("*");
        return -1;
    }

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
    }

    UCDatasetUtil* ucd = new UCDatasetUtil(host , port);
    std::string commond_1 = argv[1];

    if(commond_1 == "check")
    {
        if(argc == 2)
        {
            ucd->check_ucd();
        }
        else
        {
            print_info("check");
        }
    }
    else if(commond_1 == "load")
    {        
        if(argc < 2)
        {
            print_info("load");
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
        }
        ucd->save_ucd(ucd_name, ucd_save_path);
    }
    else if(commond_1 == "delete")
    {
        if(argc != 3)
        {
            print_info("delete");
        }
        else
        {
            std::string std_name = argv[2];
            ucd->delete_ucd(std_name);
        }
    }
    else if(commond_1 == "save")
    {
        if((argc != 5) && (argc != 6))
        {
            print_info("save");
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
            if(save_mode.size() != 3)
            {
                std::cout << "save_mode illeagal, need save_mode such as 111 | 101  " << save_dir << std::endl;
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
            
            if(save_mode[2] == '0')
            {
                need_json = false;
            }
            else
            {
                need_json = true;
            }
            // load
            ucd->json_path = json_path;
            ucd->save_img_xml_json(save_dir, need_img, need_xml, need_json, need_count);
        }
    }
    else if(commond_1 == "upload")
    {
        std::string ucd_path, assign_ucd_name;
        if(argc == 3)
        {
            ucd_path = argv[2];
            ucd->upload_ucd(ucd_path);
        }
        else if(argc == 4)
        {
            ucd_path = argv[2];
            assign_ucd_name = argv[3];
            ucd->upload_ucd(ucd_path, assign_ucd_name);
        }
        else
        {
            print_info("upload");
        }
    }        
    else if(commond_1 == "info")
    {
        if(argc != 3)
        {
            print_info("info");
        }

        std::string json_path = argv[2];
        UCDataset * ucd_info = new UCDataset(json_path);
        ucd_info->parse_json_info();
        ucd_info->print_json_info();
        delete ucd_info;
    }
    else if(commond_1 == "from")
    {
        if(argc == 4)
        {
            std::string img_path = argv[2];
            std::string ucd_name = argv[3];
            ucd->get_ucd_from_img_dir(img_path, ucd_name);
        }
        else
        {
            print_info("from");
        }
    }
    else if(commond_1 == "show")
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
        }
        // 展示所有的下载路径，为了方便单张图片的信息下载查看，可以使用 curl 下载单个 uc 对应的信息
        std::cout << "load img      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".jpg" << std::endl;
        std::cout << "load xml      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".xml" << std::endl;
        std::cout << "load json     : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".json" << std::endl;
        std::cout << "load ucd      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/ucd/{ucd_name}.json" << std::endl;
        std::cout << "check         : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/ucd/check" << std::endl;
    }
    else if(commond_1 == "diff")
    {
        if(argc == 4)
        {
            std::string ucd_path_1 = argv[2];
            std::string ucd_path_2 = argv[3];
            ucd->ucd_diff(ucd_path_1, ucd_path_2);
        }
        else
        {
            print_info("diff");
        }
    }
    else if(commond_1 == "merge")
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
            ucd->merge_ucds(save_path, ucd_path_vector);
        }
        else
        {
            print_info("merge");
        }
    }
    else if(commond_1 == "minus")
    {
        if(argc == 5)
        {
            std::string ucd_save_path = argv[2];
            std::string ucd_path_1 = argv[3];
            std::string ucd_path_2 = argv[4];
            ucd->ucd_minus(ucd_save_path, ucd_path_1, ucd_path_2);
        }
        else
        {
            print_info("minus");
        }
    }
    else if(commond_1 == "meta")
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
            std::cout << "-----------------------------" << std::endl;
        }
        else
        {
            print_info("meta");
        }
    }
    else if(commond_1 == "set")
    {
        if(argc == 4)
        {
            xini_file_t xini_write(config_path);
            std::string option = argv[2]; 

            if(option == "host")
            {
                std::string host = argv[3];
                xini_write["server"]["host"] = host;
            }
            else if(option == "port")
            {
                int port = std::stoi(argv[3]);
                xini_write["server"]["port"] = port;      
            }
            else if(option == "sql_host")
            {
                std::string host = argv[3];
                xini_write["sql"]["host"] = host;
            }
            else if(option == "sql_port")
            {
                int port = std::stoi(argv[3]);
                xini_write["sql"]["port"] = port;   
            }
            else if(option == "sql_user")
            {
                std::string user = argv[3];
                xini_write["sql"]["user"] = user;
            }
            else if(option == "sql_pwd")
            {
                std::string pwd = argv[3];
                xini_write["sql"]["pwd"] = pwd;
            }
            else if(option == "sql_db")
            {
                std::string db = argv[3];
                xini_write["sql"]["db"] = db;
            }
            else
            {
                print_info("set");
            }
            xini_write.dump(config_path);   
        }
        else
        {
            print_info("set");
        }
    }
    else if(commond_1 == "rename_img")
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
            }
        }
        else
        {
            print_info("rename_img");
        }
    }
    else if(commond_1 == "rename_img_xml")
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
            }
        }
        else
        {
            print_info("rename_img_xml");
        }
    }
    else
    {
        print_info("*");
        return -1;
    }

    delete ucd;

	return 1;
}
