

#include <iostream>
#include <fstream>
#include <set>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
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
#include <algorithm>
#include <nlohmann/json.hpp>
#include "include/lablelmeObj.hpp"
#include <httplib.h>
#include "./include/yoloDete.hpp"
using namespace httplib;
using json = nlohmann::json;
using namespace jotools;
using namespace std;


// 解决上传大数据会报错的问题 refer : https://blog.csdn.net/harry49/article/details/115763383


int main(int argc, char ** argv)
{

    // // -------------------------------------------
    // std::string log_dir = "/usr/logs";
    // std::string model_name = "prebase_yolo5_0_4_0";
    // std::string config_path = "/home/server/config.ini";
    // std::string section = "prebase";
    // std::string host = "192.168.3.111";
    // int port = 11101;
    // // -------------------------------------------

    std::string log_dir     = argv[1];
    std::string model_name  = argv[2];
    std::string config_path = argv[3];
    std::string section     = argv[4];
    std::string host        = argv[5];
    int port                = std::stoi(argv[6]);

    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "log_dir       : " << log_dir << std::endl;
    std::cout << "model_name    : " << model_name << std::endl;
    std::cout << "config_path   : " << config_path << std::endl;
    std::cout << "section       : " << section << std::endl;
    std::cout << "host          : " << host << std::endl;
    std::cout << "port          : " << port << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "ucd " << log_dir << " " << model_name << "  " << config_path << " " << section << " " << host << " " << port << " " << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    std::string load_ucd_dir    = log_dir + "/" + "load_ucd_dir";
    std::string save_xml_dir    = log_dir + "/" + "save_xml_dir";
    std::string temp_dir        = log_dir + "/" + "ucd_cache";

    if(! is_dir(log_dir))
    {
        mkdir(log_dir.c_str(), S_IRWXU);
    }

    if(! is_dir(load_ucd_dir))
    {
        mkdir(load_ucd_dir.c_str(), S_IRWXU);
    }

    if(! is_dir(save_xml_dir))
    {
        mkdir(save_xml_dir.c_str(), S_IRWXU);
    }

    if(! is_dir(temp_dir))
    {
        mkdir(temp_dir.c_str(), S_IRWXU);
    }

    Yolov5 model(config_path, section);
    model.model_restore();
    UCDatasetUtil* ucd_util = new UCDatasetUtil(host, port, temp_dir);
    bool is_waiting = false;
    while(true)
    {
        // scan ucd_dir
        std::set<std::string> json_suffix {".json"};
        std::vector<std::string> ucd_path_vector = get_all_file_path(load_ucd_dir, json_suffix);

        for(int i=0; i<ucd_path_vector.size(); i++)
        {
            is_waiting = false;
            UCDataset* ucd = new UCDataset(ucd_path_vector[i]);
            ucd->parse_ucd();
            std::vector<std::string> uc_vector = ucd->uc_list;
            int ucd_count = uc_vector.size();
            std::cout << "start detect, uc_count : " << ucd_count << std::endl;
            for(int j=0; j<uc_vector.size(); j++)
            {
                std::string uc = uc_vector[j];
                std::string save_img_path = ucd_util->cache_img_dir + "/" + uc + ".jpg";
                std::string each_save_path = save_xml_dir + "/" + uc + "_" + model_name + ".xml";
                // if has_dete
                if(! is_file(each_save_path))
                {
                    ucd_util->load_img(ucd_util->cache_img_dir, {uc});
                    cv::Mat pic = cv::imread(save_img_path);
                    DeteRes dete_res = model.dete(pic);
                    dete_res.save_to_xml(each_save_path);
                    std::cout << j << "/" << ucd_count << ", dete img : "<< uc << std::endl;
                }
                else
                {
                    std::cout << j << "/" << ucd_count << ", ignore img : "<< uc << std::endl;
                }
                remove(save_img_path.c_str());
            }
            delete ucd;
            remove(ucd_path_vector[i].c_str());
        }
        if(is_waiting == false)
        {
            time_t now = time(0);
            char* dt = ctime(&now);
            std::cout << "waiting from : " << dt << std::endl;
            is_waiting = true;
        }
        sleep(5);
    }
    delete ucd_util;
}

