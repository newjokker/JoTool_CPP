

#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <set>
#include <iterator>
#include "include/fileOperateUtil.hpp"
#include "include/ucDatasetUtil.hpp"
#include "include/deteRes.hpp"

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

void UCDataset::parse_json_info(bool parse_xml_info)
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
    if(uc_list != nullptr)
    { 
        UCDataset::uc_list = uc_list; 
        UCDataset::unique();
    }
    
    // parse xml_info
    if(parse_xml_info)
    {
        auto xml_info = data["xml_info"];
        if(xml_info != nullptr)
        {
            // 读取存储的 xml info， 存放到 字典中，{uc: [x1, y1, x2, y2, conf, tag]}
            UCDataset::xml_info = xml_info;
        }
    }
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
    std::cout << "label_used        : " << UCDataset::label_used.size() <<std::endl;
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
        {"label_used", UCDataset::label_used},
        {"uc_list", UCDataset::uc_list},
        {"xml_info", {}}
    };

    // xml info
    nlohmann::json xml_data;
    xml_data =  UCDataset::xml_info;
    json_info["xml_info"] = xml_data;

    std::ofstream o(save_path);
    // 不加 std::setw(4) 就不是格式化输出，都显示在一行
    o << std::setw(4) << json_info << std::endl;
    // o << json_info << std::endl;
}

void UCDataset::unique()
{
    std::set<std::string> uc_set(UCDataset::uc_list.begin(), UCDataset::uc_list.end());
    UCDataset::uc_list.assign(uc_set.begin(), uc_set.end());
}


UCDatasetUtil::UCDatasetUtil(std::string host, int port, std::string cache_dir)
{
    UCDatasetUtil::host = host;
    UCDatasetUtil::port = port;
    UCDatasetUtil::root_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
    UCDatasetUtil::cache_dir = cache_dir;
    if(is_dir(cache_dir))
    {
        UCDatasetUtil::cache_img_dir = UCDatasetUtil::cache_dir + "/" + "img_cache";
        if(! is_dir(UCDatasetUtil::cache_img_dir))
        {
            create_folder(UCDatasetUtil::cache_img_dir);
        }
    }
}

void UCDatasetUtil::save_img_xml_json(std::string save_dir, bool need_img, bool need_xml, int need_count)
{
    if(! is_dir(save_dir))
    {
        std::cout << "save dir not exists : " << save_dir << std::endl;
        throw "save dir not exists";
    }
    // 
    std::string save_img_dir;
    if(is_dir(UCDatasetUtil::cache_img_dir))
    {
        save_img_dir = cache_img_dir;
    }
    else
    {
        save_img_dir = save_dir + "/" + "img";
    }
    std::string save_xml_dir = save_dir + "/" + "xml";
    std::string save_json_dir = save_dir + "/" + "json";

    // 
    if(need_img){create_folder(save_img_dir);}
    if(need_xml){create_folder(save_xml_dir);}

    //
	// std::ifstream jsfile(UCDatasetUtil::json_path);
    // json data = json::parse(jsfile); 

    UCDataset* ucd = new UCDataset(UCDatasetUtil::json_path);
    ucd->parse_json_info();

    std::string each_uc;
    std::string img_url, xml_url, json_url;
    std::string save_img_path, save_xml_path, save_json_path;

    // for(int i=0; i<data["uc_list"].size(); i++)
    for(int i=0; i<ucd->uc_list.size(); i++)
    {
            if((need_count == -1) || (i < need_count))
            {
            each_uc = ucd->uc_list[i];
            //
            img_url = "/file/" + each_uc + ".jpg";
            xml_url = "/file/" + each_uc + ".xml";
            //
            save_img_path = save_img_dir + "/" + each_uc + ".jpg";
            save_xml_path = save_xml_dir + "/" + each_uc + ".xml";
            
            if(need_img){ UCDatasetUtil::load_file(img_url, save_img_path, i); }
            if(need_xml){ UCDatasetUtil::load_file(xml_url, save_xml_path, i); }
        }
    }
    delete ucd;
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

        if(res->status == 200)
        {
            std::ofstream out;
            out.open(save_path, std::ios_base::binary | std::ios::out);
            out<<res->body;
            out.close();
        }
        else
        {
            std::cout << "load error : " << url << std::endl;
            // throw "load error : " + url;
        }
    }
}

static bool is_uc(std::string uc)
{
    if(uc.size() != 7){ return false; }
    if(((int)uc[0] < (int)'C') || ((int)uc[0] > int('K'))) { return false; }
    if(((int)uc[1] < (int)'a') || ((int)uc[1] > int('z'))) { return false; }
    if(((int)uc[2] < (int)'a') || ((int)uc[2] > int('z'))) { return false; }
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
            std::cout << img_path_vector[i] << std::endl;
            ucd->uc_list.push_back(uc);
        }
    }
    ucd->save_to_json(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_img_xml_dir(std::string img_dir, std::string xml_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> img_path_vector = get_all_file_path_recursive(img_dir, suffix);

    UCDataset* ucd = new UCDataset();
    std::string uc, xml_path;

    for(int i=0; i<img_path_vector.size(); i++)
    {
        uc = get_file_name(img_path_vector[i]);
        xml_path = xml_dir + "/" + uc + ".xml";
        if(is_uc(uc) && is_file(xml_path))
        {
            // std::cout << img_path_vector[i] << std::endl;
            ucd->uc_list.push_back(uc);
            // read xml info
            jotools::DeteRes* dete_res = new jotools::DeteRes(xml_path);
            std::vector< std::vector<std::string> > xml_info; 
            for(int j=0; j<dete_res->size(); j++)
            {
                std::vector<std::string> each_xml_info;
                each_xml_info.push_back(std::to_string(dete_res->alarms[j].x1));        
                each_xml_info.push_back(std::to_string(dete_res->alarms[j].y1));        
                each_xml_info.push_back(std::to_string(dete_res->alarms[j].x2));        
                each_xml_info.push_back(std::to_string(dete_res->alarms[j].y2));        
                each_xml_info.push_back(std::to_string(dete_res->alarms[j].conf));        
                each_xml_info.push_back(dete_res->alarms[j].tag);
                xml_info.push_back(each_xml_info); 
                // std::cout << uc << std::endl;       
            }
            ucd->xml_info[uc] = xml_info;
            delete dete_res;
        }
    }
    ucd->save_to_json(ucd_path);
    delete ucd;
}

void UCDatasetUtil::merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector)
{
    // ucd path
    for(int i=0; i<ucd_path_vector.size(); i++)
    {
        if(! is_file(ucd_path_vector[i]))
        {
            std::cout << "ucd path not exists : " << save_path << std::endl;
            throw "ucd path not exists";
        }
    }
    // merge 
    UCDataset* merge = new UCDataset();
    std::set<std::string> uc_set, used_label_set;
    for(int i=0; i<ucd_path_vector.size(); i++)
    {
        UCDataset* ucd = new UCDataset(ucd_path_vector[i]);
        ucd->parse_json_info();
        // uc
        for(int j=0; j<ucd->uc_list.size(); j++)
        {
            uc_set.insert(ucd->uc_list[j]);
        }
        // used label
        for(int j=0; j<ucd->label_used.size(); j++)
        {
            used_label_set.insert(ucd->label_used[j]);
        }
        delete ucd;
    }
    // uc list
    std::set<std::string>::iterator iter_uc = uc_set.begin();
    while(iter_uc != uc_set.end())
    {
        merge->uc_list.push_back(iter_uc->data());
        iter_uc ++;
    }
    // label used
    std::set<std::string>::iterator iter_label = used_label_set.begin();
    while(iter_label != used_label_set.end())
    {
        merge->label_used.push_back(iter_label->data());
        iter_label ++;
    }
    merge->save_to_json(save_path);
    delete merge;
}

void UCDatasetUtil::ucd_diff(std::string ucd_path_1, std::string ucd_path_2)
{
    UCDataset* ucd1 = new UCDataset(ucd_path_1);
    ucd1->parse_json_info();
    UCDataset* ucd2 = new UCDataset(ucd_path_2);
    ucd2->parse_json_info();
    std::vector<std::string> uc_intersection;
    // set_intersection
    std::set<std::string> uc_set1(ucd1->uc_list.begin(), ucd1->uc_list.end());
    std::set<std::string> uc_set2(ucd2->uc_list.begin(), ucd2->uc_list.end());
    std::set_intersection(uc_set1.begin(), uc_set1.end(), uc_set2.begin(), uc_set2.end(), std::inserter(uc_intersection, uc_intersection.begin()));
    // print
    std::cout << "----------------------" << std::endl;
    std::cout << "A  : " << ucd1->uc_list.size() - uc_intersection.size() << std::endl;
    std::cout << "AB : " << uc_intersection.size() << std::endl;
    std::cout << "B  : " << ucd2->uc_list.size() - uc_intersection.size() << std::endl;
    std::cout << "----------------------" << std::endl;
    // delete
    delete ucd1;
    delete ucd2;
}

void UCDatasetUtil::ucd_minus(std::string save_path, std::string ucd_path_1, std::string ucd_path_2)
{
    UCDataset* ucd1 = new UCDataset(ucd_path_1);
    ucd1->parse_json_info();
    UCDataset* ucd2 = new UCDataset(ucd_path_2);
    ucd2->parse_json_info();
    UCDataset* ucd_res = new UCDataset();
    std::vector<std::string> uc_difference;
    // set_intersection
    std::set<std::string> uc_set1(ucd1->uc_list.begin(), ucd1->uc_list.end());
    std::set<std::string> uc_set2(ucd2->uc_list.begin(), ucd2->uc_list.end());
    std::set_difference(uc_set1.begin(), uc_set1.end(), uc_set2.begin(), uc_set2.end(), std::inserter(uc_difference, uc_difference.begin()));
    // save
    ucd_res->uc_list = uc_difference;
    ucd_res->save_to_json(save_path);
    // delete
    delete ucd1;
    delete ucd2;
    delete ucd_res;
}

void UCDatasetUtil::save_xml(std::string save_dir, int get_count)
{
    // to 创建一个 xml 路径 xml_from_ucd
    
    std::string save_xml_dir = save_dir + "/" + "xml_from_ucd";

    if(! is_dir(save_xml_dir))
    {
        create_folder(save_xml_dir);
    }

    //
    UCDataset* ucd = new UCDataset(UCDatasetUtil::json_path);
    ucd->parse_json_info(true);

    std::string save_xml_path, uc;
    std::string tag;
    int x1, x2, y1, y2;
    float conf;

    for(int i=0; i<ucd->uc_list.size(); i++)
    {
        if(get_count == 0)
        { 
            break; 
        }
        get_count-- ;

        uc = ucd->uc_list[i];
        save_xml_path = save_xml_dir + "/" + uc + ".xml";
        //
        jotools::DeteRes* dete_res = new jotools::DeteRes();
        std::vector< std::vector< std::string > > xml_info = ucd->xml_info[uc];
        for(int i=0; i<xml_info.size(); i++)
        {
            x1 = std::stoi(xml_info[i][0]);
            y1 = std::stoi(xml_info[i][1]);
            x2 = std::stoi(xml_info[i][2]);
            y2 = std::stoi(xml_info[i][3]);
            conf = std::stof(xml_info[i][4]);
            tag = xml_info[i][5];
            dete_res->add_dete_obj(x1, y1, x2, y2, conf, tag);
        }
        dete_res->save_to_xml(save_xml_path);
        delete dete_res;
    }
    delete ucd;
}

bool UCDatasetUtil::is_ucd_path(std::string ucd_path)
{
    if((! is_file(ucd_path)) || (ucd_path.substr(ucd_path.size()-5, ucd_path.size()) != ".json"))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void UCDatasetUtil::count_ucd_tags(std::string ucd_path)
{
    if(! UCDatasetUtil::is_ucd_path(ucd_path))
    {
        std::cout << "ucd path not exists : " << ucd_path << std::endl;
    }

    std::map< std::string, int > count_map;
    int uc_count = 0;
    int dete_obj_count=0; 
    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_json_info(true);

    std::string each_tag;
    auto iter = ucd->xml_info.begin();
    while(iter != ucd->xml_info.end())
    {
        uc_count += 1;
        for(int i=0; i<iter->second.size(); i++)
        {
            // x1, y1, x2, y2, conf, tag
            dete_obj_count += 1;
            each_tag = iter->second[i][5];
            if(count_map.count(each_tag) == 0)
            {
                count_map[each_tag] = 1;
            }
            else
            {
                count_map[each_tag] += 1;
            }
        }
        iter++;
    }
    // print statistics res
    auto iter_count = count_map.begin();
    std::cout << "-------------------------------" << std::endl;
    while(iter_count != count_map.end())
    {
        std::cout << iter_count->first << " : " << iter_count->second << std::endl;
        iter_count ++;
    }
    std::cout << "number of uc  : " << uc_count << std::endl;
    std::cout << "number of tag : " << dete_obj_count << std::endl;
    std::cout << "-------------------------------" << std::endl;
}



