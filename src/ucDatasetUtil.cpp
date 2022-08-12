

#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <./include/ucDatasetUtil.hpp>
#include "include/fileOperateUtil.hpp"
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <set>

using json = nlohmann::json;


// CPP 写服务端 refer : https://blog.csdn.net/canlynetsky/article/details/119083255


UCDataset::UCDataset(std::string json_path)
{
    UCDataset::dataset_name = "";
    UCDataset::model_name = "";
    UCDataset::model_version = "";
    std::vector<std::string> label_used;
    UCDataset::add_time = -1;
    UCDataset::update_time = -1;
    UCDataset::describe = "";
    UCDataset::json_path = json_path;
}

void UCDataset::parse_json_info()
{
    if(! is_file(UCDataset::json_path))
    {
        std::cout << "json path not exists : " << UCDataset::json_path << std::endl;
        throw "json path not exists";
    }
    
    std::ifstream jsfile(UCDataset::json_path);
    json data = json::parse(jsfile); 

    auto dataset_name = data["dataset_name"];
    auto model_name = data["model_name"];
    auto model_version = data["model_version"];
    auto add_time = data["add_time"];
    auto update_time = data["update_time"];
    auto describe = data["describe"];
    auto label_used = data["label_used"];
    auto uc_list = data["uc_list"];

    if(dataset_name != nullptr){ UCDataset::dataset_name = dataset_name; }
    if(model_name != nullptr){ UCDataset::model_name = model_name; }
    if(model_version != nullptr){ UCDataset::model_version = model_version; }
    if(add_time != nullptr){ UCDataset::add_time = add_time; }
    if(update_time != nullptr){ UCDataset::update_time = update_time; }
    if(describe != nullptr){ UCDataset::describe = describe; }
    if(label_used != nullptr){ UCDataset::label_used = label_used; }
    if(uc_list != nullptr){ UCDataset::uc_list = uc_list; }

}

void UCDataset::print_json_info()
{
    std::cout << "--------------------------------" << std::endl;
    std::cout << "dataset_name      : " << UCDataset::dataset_name << std::endl;
    std::cout << "uc count          : " << UCDataset::uc_list.size() << std::endl;
    std::cout << "model_name        : " << UCDataset::model_name << std::endl;
    std::cout << "model_version     : " << UCDataset::model_version << std::endl;
    std::cout << "add_time          : " << UCDataset::add_time << std::endl;
    std::cout << "update_time       : " << UCDataset::update_time << std::endl;
    std::cout << "describe          : " << UCDataset::describe << std::endl;
    std::cout << "label_used        : " << UCDataset::model_version << std::endl;
    for(int i=0; i<UCDataset::label_used.size(); i++)
    {
        std::cout << "  * " << UCDataset::label_used[i] << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
}

void UCDataset::save_to_json(std::string save_path)
{
    nlohmann::json json_info = {
        {"dataset_name", UCDataset::dataset_name},
        {"model_name", UCDataset::model_name},
        {"model_version", UCDataset::model_version},
        {"add_time", UCDataset::add_time},
        {"update_time", UCDataset::update_time},
        {"describe", UCDataset::describe},
    };

    json_info["label_used"] = UCDataset::label_used;
    json_info["uc_list"] = UCDataset::uc_list;

    std::ofstream o(save_path);
    // 不加 std::setw(4) 就不是格式化输出，都显示在一行
    o << std::setw(4) << json_info << std::endl;
    // o << json_info << std::endl;
}

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

static bool is_uc(std::string uc)
{
    if(uc.size() != 7){ return false; }

    // if((uc[0] != 'C') || (uc[0] != 'D') || (uc[0] != 'E') || (uc[0] != 'F')) { return false; }

    if(((int)uc[0] < (int)'C') || ((int)uc[0] > int('K'))) { return false; }


    return true;
}

void UCDatasetUtil::get_ucd_from_img_dir(std::string img_dir, std::string ucd_path)
{

    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> img_path_vector = get_all_file_path_recursive(img_dir, suffix);

    UCDataset* ucd = new UCDataset();

    std::string uc;
    for(int i=0; i<img_path_vector.size(); i++)
    {
        uc = get_file_name(img_path_vector[i]);

        if(is_uc(uc))
        {
            ucd->uc_list.push_back(uc);
        }

        std::cout << img_path_vector[i] << std::endl;
    }

    ucd->save_to_json(ucd_path);
    delete ucd;

}