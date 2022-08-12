
#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include "./include/crow_all.h"
#include "./include/strToImg.hpp"
#include "./deteRes.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "./include/operateDeteRes.hpp"
#include "./include/pystring.h"
#include "./include/fileOperateUtil.hpp"
// #include "include/load_img.hpp"
#include "include/ucDatasetUtil.hpp"

using namespace jotools;
using namespace std;



// 提供一个图床服务，图片专门放在一个服务器中 (放在多个服务器中，分担压力)

// todo 增加将文件夹中的图片转为 ucDataset.json 的功能

// 当操作之后不要生成文件

// 创建的文件夹要最高的权限

// ucd load 可以指定保存文件夹 或者保存路径

// 单张图片的下载

// 设置配置文件，host port

// img -> ucd.json

// 完善报错机制

// 方便的查看 ucd 相关的信息

// 时间使用更加方便的方式进行展示


int main(int argc, char ** argv)
{
    if ((argc < 2))
    {
        std::cout << "need parameter number >= 1 get : " << argc-1 << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "uc save json_path save_dir save_mode(111|100) {need_count}" << std::endl;
        std::cout << "ucd check" << std::endl;
        std::cout << "ucd delete ucd_name" << std::endl;
        std::cout << "ucd load ucd_name save_path" << std::endl;
        std::cout << "ucd upload ucd_path {ucd_name}" << std::endl;
        //
        std::cout << "ucd from img_dir ucd_save_path" << std::endl;
        return -1;
    }

    UCDatasetUtil* ucd = new UCDatasetUtil("192.168.3.111" , 11101);

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
            // 指定的是个文件夹的话直接用 ucd_name 保存在指定的文件夹中
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
        UCDataset * ucd = new UCDataset(json_path);

        ucd->parse_json_info();
        ucd->print_json_info();

        delete ucd;
    }
    else if(commond_1 == "from")
    {
        std::cout << "from 功能还未开发好" << std::endl;
    }
    else
    {
        std::cout << "need parameter number > 2 get : " << argc-1 << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "uc save json_path save_dir save_mode(111|100)" << std::endl;
        std::cout << "ucd check" << std::endl;
        std::cout << "ucd delete ucd_name" << std::endl;
        std::cout << "ucd upload ucd_path {ucd_name}" << std::endl;
        std::cout << "ucd load ucd_name save_path" << std::endl;
        return -1;
    }

    delete ucd;

	return 1;
	
}
