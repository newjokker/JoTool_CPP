

#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <./include/ucDatasetUtil.hpp>
// #include "include/load_img.hpp"
#include "include/fileOperateUtil.hpp"
#include <nlohmann/json.hpp>
#include <httplib.h>



using json = nlohmann::json;



UCDatasetUtil::UCDatasetUtil(std::string host, int port)
{
    UCDatasetUtil::host = host;
    UCDatasetUtil::port = port;
    UCDatasetUtil::root_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
}


void UCDatasetUtil::save_img_xml_json(std::string save_dir, bool need_img, bool need_xml, bool need_json)
{

    if(! is_dir(save_dir))
    {
        std::cout << "save dir not exists : " << save_dir << std::endl;
        throw "save dir not exists";
    }

    // 
    std::string save_img_dir = save_dir + "/" + "img";
    std::string save_xml_dir = save_dir + "/" + "xml";
    std::string save_json_dir = save_dir + "/" + "json";

    // 
    if(need_img){create_folder(save_img_dir);}
    if(need_xml){create_folder(save_xml_dir);}
    if(need_json){create_folder(save_json_dir);}

    //
	std::ifstream jsfile(UCDatasetUtil::json_path);
    json data = json::parse(jsfile); 

    std::string each_uc;
    std::string img_url, xml_url, json_url;
    std::string save_img_path, save_xml_path, save_json_path;

    for(int i=0; i<data["uc_list"].size(); i++)
    {
        each_uc = data["uc_list"][i];
        //
        img_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port) + "/file/" + each_uc + ".jpg";
        xml_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port) + "/file/" + each_uc + ".xml";
        json_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port) + "/file/" + each_uc + ".json";
        //
        save_img_path = save_img_dir + "/" + each_uc + ".jpg";
        save_xml_path = save_xml_dir + "/" + each_uc + ".xml";
        save_json_path = save_json_dir + "/" + each_uc + ".json";
        
        // if(need_img){ download_file(img_url, save_img_path); }
        // if(need_xml){ download_file(xml_url, save_xml_path); }
        // if(need_json){ download_file(json_url, save_json_path); }

        std::cout << img_url << std::endl;
        std::cout << xml_url << std::endl;
        std::cout << json_url << std::endl;
    }
}


void UCDatasetUtil::check_ucd()
{
    std::string check_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
    httplib::Client cli(check_url);
    auto res = cli.Get("/ucd/check");
    
    json data = json::parse(res->body);
    // customer
    for(int i=0; i<data["official"].size(); i++)
    {
        std::cout << "official : " << data["official"][i] << std::endl;
    }
    // official
    for(int i=0; i<data["customer"].size(); i++)
    {
        std::cout << "customer : " << data["customer"][i] << std::endl;
    }
}

void UCDatasetUtil::delete_ucd(std::string std_name)
{
    httplib::Client cli(UCDatasetUtil::root_url);
    auto res = cli.Delete("/ucd/delete/" + std_name + ".json");
}


void UCDatasetUtil::save_ucd(std::string ucd_name, std::string save_path)
{
    UCDatasetUtil::load_file("/ucd/" + ucd_name, save_path);
}

void UCDatasetUtil::upload_ucd(std::string ucd_path, std::string assign_ucd_name)
{
    // httplib::Client cli(UCDatasetUtil::root_url);

    // auto res = cli.Post("ucd/upload");
    // std::ofstream out;
    // out.open(ucd_path, std::ios_base::binary | std::ios::in);
    // out<<res->body;
    // out.close();
}

// void UCDatasetUtil::save_ucd(std::string ucd_name, std::string assign_ucd_name)
// {
//     httplib::Client cli(UCDatasetUtil::root_url);

//     auto res = cli.Post("ucd/" + ucd_name);
    
//     std::ofstream out;
//     out.open(ucd_path, std::ios_base::binary | std::ios::in);
//     out<<res->body;
//     out.close();
// }


void UCDatasetUtil::load_file(std::string url, std::string save_path)
{
    // refer : https://blog.csdn.net/harry49/article/details/115763383

    httplib::Client cli(UCDatasetUtil::root_url);

    auto res = cli.Get(url);
    std::ofstream out;
    out.open(save_path, std::ios_base::binary | std::ios::out);
    out<<res->body;
    out.close();
}
