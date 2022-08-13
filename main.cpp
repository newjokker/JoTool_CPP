
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

using namespace jotools;
using namespace std;


// nginx 负载均衡，可以在风火轮上部署，转到 111 和 209 服务器上

// 完善路径拼接（c++多一些 // 不会造成路径错误，少一些就会报错）


void print_info()
{
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "下载 图片|标注|元信息, ucd save json_path save_dir save_mode(111|100) {need_count}" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "查看所有在线数据集, ucd check" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "查看所有下载路径, ucd show {uc}" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "删除在线数据集,无法删除官方数据集 ucd delete ucd_name" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "下载在线数据集 ucd load ucd_name save_path|save_dir " << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "上传数据集到网络 ucd upload ucd_path {ucd_name}" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "本地文件生成数据集 ucd from img_dir ucd_save_path" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "查看数据集信息 ucd info ucd_path" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
}


int main(int argc, char ** argv)
{
    if ((argc < 2))
    {
        std::cout << "need parameter number >= 1 get : " << argc-1 << std::endl;
        print_info();
        return -1;
    }

    std::string host = "192.168.3.111";
    int port = 11101;
    std::string config_path;

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
        host = (const std::string &)xini_file["info"]["host"];
        port = (const int &)xini_file["info"]["port"];
    }

    UCDatasetUtil* ucd = new UCDatasetUtil(host , port);
    std::string commond_1 = argv[1];

    if(commond_1 == "check")
    {
        ucd->check_ucd();
    }
    else if(commond_1 == "load")
    {        
        if(argc < 2)
        {
            std::cout << "ucd load ucd_name {ucd_path}" << std::endl;
            throw "ucd load ucd_name {ucd_path}";
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
            std::cout << "ucd load ucd_name {ucd_path}" << std::endl;
            throw "ucd load ucd_name {ucd_path}";
        }
        ucd->save_ucd(ucd_name, ucd_save_path);
    }
    else if(commond_1 == "delete")
    {
        if(argc != 3)
        {
            std::cout << "ucd delete ucd_name " << std::endl;
            throw "ucd delete ucd_name";
        }
        else
        {
            std::string std_name = argv[2];
            ucd->delete_ucd(std_name);
        }
    }
    else if(commond_1 == "save")
    {
        // save img | xml | json
        if((argc != 5) && (argc != 6))
        {
            std::cout << "uc save json_path save_path save_mode" << std::endl;
            throw "ucd delete ucd_name";
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
            std::cout << "ucd upload json_path {assign_ucd_name}" << std::endl;
            throw "ucd upload error";
        }
    }        
    else if(commond_1 == "info")
    {
        if(argc != 3)
        {
            std::cout << "ucd info json_path" << std::endl;
            throw "ucd info error";
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
    }
    else if(commond_1 == "show")
    {
        std::string uc;
        if(argc >= 3)
        {
            uc = argv[2];
        }
        else
        {
            uc = "{UC}";
        }
        // 展示所有的下载路径，为了方便单张图片的信息下载查看
        std::cout << "load img      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".jpg" << std::endl;
        std::cout << "load xml      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".xml" << std::endl;
        std::cout << "load json     : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/file/" + uc + ".json" << std::endl;
        std::cout << "load ucd      : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/ucd/{ucd_name}.json" << std::endl;
        std::cout << "check         : http://" + ucd->host + ":" + std::to_string(ucd->port) + "/ucd/check" << std::endl;
    }
    else
    {
        print_info();
        return -1;
    }

    delete ucd;

	return 1;
	
}
