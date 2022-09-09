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
#include <mysql/mysql.h>
#include "./include/yoloDete.hpp"
using namespace httplib;
using json = nlohmann::json;
using namespace jotools;
using namespace std;


// todo 查看需要插入的表是否存在，不存在的话直接创建一个表

// 当检测到一半报错的时候就会出现 xml 结果和 数据库中的结果不对应的情况，数据库中的结果会少一些，因为到最后才进行插入

// 解决上传大数据会报错的问题 refer : https://blog.csdn.net/harry49/article/details/115763383


void dete_res_to_mysql(std::map<std::string, std::string> xml_map, std::string table_name)
{
    // sql info 
    int sql_port = 3306;
    std::string sql_host = "192.168.3.101";
    std::string sql_user = "root";
    std::string sql_pwd = "root123";
    std::string sql_db = "dete_res";

    MYSQL *conn;
    int res;
    conn = mysql_init(NULL);  

    // connect
    if (!mysql_real_connect(conn, sql_host.c_str(), sql_user.c_str(), sql_pwd.c_str(), sql_db.c_str(), sql_port, NULL, 0))
    {
        std::cout << "sql connect error" << std::endl;
        throw "sql connect error";
    }    

    // insert
    auto iter = xml_map.begin();
    while(iter != xml_map.end())
    {
        std::string uc = iter->first;
        std::string xml_path = iter->second;
        DeteRes * dete_res = new DeteRes(xml_path);
        for(int j=0; j<dete_res->alarms.size(); j++)
        {
            DeteObj obj = dete_res->alarms[j];
            std::string serarch_str = "INSERT INTO " + table_name + " (uc, tag, x1, y1, x2, y2, conf) VALUES ('" + uc + "', " + "'" + obj.tag + "',"  + std::to_string(obj.x1) +  "," + std::to_string(obj.y1) + "," + 
                                                                                                std::to_string(obj.x2) + "," + std::to_string(obj.y2) + ", " + std::to_string(obj.conf) +  ");";
            res = mysql_query(conn, serarch_str.c_str());
            if(res)                                
            {
                std::cout << "prebase sql insert error" << std::endl;
                std::cout << serarch_str << std::endl;
            }
        }
        delete dete_res;
        iter++;
    }
    mysql_close(conn);  
}



int main_dete_server(int argc, char ** argv)
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
    std::string dete_logs_dir   = log_dir + "/" + "dete_logs";

    mkdir(log_dir.c_str(), S_IRWXU);
    mkdir(load_ucd_dir.c_str(), S_IRWXU);
    mkdir(save_xml_dir.c_str(), S_IRWXU);
    mkdir(temp_dir.c_str(), S_IRWXU);
    mkdir(dete_logs_dir.c_str(), S_IRWXU);

    // 
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
            ofstream ofs;
            std::string ucd_name = get_file_name(ucd_path_vector[i]);
            
            time_t ucd_start_time = time(0);
            char tmp[32] = { NULL };
            strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S", localtime(&ucd_start_time));
            std::string date(tmp);
            std::string log_path = dete_logs_dir + "/" + ucd_name + "_" + date + ".log";

            std::map<std::string, std::string> xml_map;
            is_waiting = false;
            UCDataset* ucd = new UCDataset(ucd_path_vector[i]);
            ucd->parse_ucd();
            std::vector<std::string> uc_vector = ucd->uc_list;
            int ucd_count = uc_vector.size();
            std::cout << "start detect, uc_count : " << ucd_count << std::endl;

            ofs.open(log_path, ios::app);
            ofs << "start detect : " << ucd_name << ", count : "  << ucd_count << std::endl;
            ofs.close();

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
                    dete_res.width = pic.cols;
                    dete_res.height = pic.rows;
                    dete_res.depth = 3;
                    dete_res.do_nms(0.5, false);
                    dete_res.save_to_xml(each_save_path);
                    xml_map[uc] = each_save_path;
                    std::cout << j << "/" << ucd_count << ", dete img : "<< uc << std::endl;
                }
                else
                {
                    std::cout << j << "/" << ucd_count << ", ignore img : "<< uc << std::endl;
                }
                remove(save_img_path.c_str());

                ofs.open(log_path, ios::app);
                ofs << "detect : " << uc << std::endl;
                ofs.close();
            }
            delete ucd;
            remove(ucd_path_vector[i].c_str());
            dete_res_to_mysql(xml_map, model_name);

            ofs.open(log_path, ios::app);
            ofs << "detect finished " << std::endl;
            ofs.close();
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

