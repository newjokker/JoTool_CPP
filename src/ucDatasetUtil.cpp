

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


// CPP 写服务端 refer : https://blog.csdn.net/canlynetsky/article/details/119083255



UCDatasetUtil::UCDatasetUtil(std::string host, int port)
{
    UCDatasetUtil::host = host;
    UCDatasetUtil::port = port;
    UCDatasetUtil::root_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
}

void UCDatasetUtil::save_img_xml_json(std::string save_dir, bool need_img, bool need_xml, bool need_json, int need_count)
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
            if((need_count == -1) || (i < need_count))
            {
            each_uc = data["uc_list"][i];
            //
            img_url = "/file/" + each_uc + ".jpg";
            xml_url = "/file/" + each_uc + ".xml";
            json_url = "/file/" + each_uc + ".json";
            //
            save_img_path = save_img_dir + "/" + each_uc + ".jpg";
            save_xml_path = save_xml_dir + "/" + each_uc + ".xml";
            save_json_path = save_json_dir + "/" + each_uc + ".json";
            
            if(need_img){ UCDatasetUtil::load_file(img_url, save_img_path, i); }
            if(need_xml){ UCDatasetUtil::load_file(xml_url, save_xml_path, i); }
            if(need_json){ UCDatasetUtil::load_file(json_url, save_json_path, i); }
        }
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
    // refer : https://stackoverflow.com/questions/64480176/uploading-file-using-cpp-httplib
    std::ifstream t_lf_img(ucd_path);
    std::stringstream buffer_lf_img;
    buffer_lf_img << t_lf_img.rdbuf();
    std::string ucd_name = get_file_name(ucd_path);
    std::string ucd_name_suffix = get_file_name_suffix(ucd_path);

    httplib::Client cliSendFiles(UCDatasetUtil::root_url);
    if(assign_ucd_name != "")
    {
        httplib::MultipartFormDataItems items = {{"json_file", buffer_lf_img.str(), ucd_name_suffix, "application/octet-stream"},{"ucd_name", assign_ucd_name}};
        auto resSendFiles = cliSendFiles.Post("/ucd/upload", items);
    }
    else
    {
        httplib::MultipartFormDataItems items = {{"json_file", buffer_lf_img.str(), ucd_name_suffix, "application/octet-stream"},{"ucd_name", ucd_name}};
        auto resSendFiles = cliSendFiles.Post("/ucd/upload", items);
    }
}

void UCDatasetUtil::load_file(std::string url, std::string save_path, int index)
{
    // refer : https://blog.csdn.net/harry49/article/details/115763383


    if(is_file(save_path))
    {
        std::cout << "* file exists ignore : " << save_path << std::endl;
    }
    else
    {
        if(index != -1)
        {
            std::cout << index << " "<< "* load " << UCDatasetUtil::root_url << url << std::endl;
        }
        else
        {
            std::cout << "* load " << UCDatasetUtil::root_url << url << std::endl;
        }
        httplib::Client cli(UCDatasetUtil::root_url);
        auto res = cli.Get(url);
        std::ofstream out;
        out.open(save_path, std::ios_base::binary | std::ios::out);
        out<<res->body;
        out.close();
    }
}
