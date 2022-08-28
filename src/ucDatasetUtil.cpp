

#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <set>
#include <iterator>
#include "include/fileOperateUtil.hpp"
#include "include/ucDatasetUtil.hpp"
#include "include/deteRes.hpp"
#include "include/pystring.h"
#include "include/deteRes.hpp"
#include "include/strToImg.hpp"
#include "include/lablelmeObj.hpp"
#include "include/easyexif.h"
#include "include/imageinfo.hpp"

using json = nlohmann::json;
using namespace jotools;


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

// UCDataset::~UCDataset()
// {
//     auto iter = UCDataset::object_info.begin();
//     while(iter != UCDataset::object_info.end())
//     {
//         for(int i=0; i<UCDataset::object_info.size(); i++)
//         {
//             delete iter->second[i];
//         }
//     }
// }

void UCDataset::parse_ucd(bool parse_shape_info)
{
    if(! is_file(UCDataset::json_path))
    {
        std::cout << "json path not exists : " << UCDataset::json_path << std::endl;
        throw "json path not exists";
    }

    // todo 原始数据的清空

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
    
    // parse shape_info
    if(parse_shape_info)
    {
        auto shapes_info = data["shapes"];
        LabelmeObjFactory obj_factory;

        if(shapes_info != nullptr)
        {
            auto iter = shapes_info.begin();
            while(iter != shapes_info.end())
            {
                std::string uc = iter.key();
                for(int i=0; i<iter.value().size(); i++)
                {
                    std::string shape_type = iter.value()[i]["shape_type"];
                    std::string label = iter.value()[i]["label"];
                    std::vector< std::vector<double> > points = iter.value()[i]["points"];
                    LabelmeObj* obj = obj_factory.CreateObj(shape_type);
                    obj->label = label;
                    obj->points = points;
                    UCDataset::object_info[uc].push_back(obj);
                }
                iter++;
            }
        }
    }
}

void UCDataset::print_ucd_info()
{
    // print statistics res
    if(is_file(UCDataset::json_path))
    {
        UCDataset::parse_ucd();
        // json 属性
        std::cout << "--------------------------------" << std::endl;
        std::cout << "dataset_name      : " << UCDataset::dataset_name << std::endl;
        std::cout << "uc_count          : " << UCDataset::uc_list.size() << std::endl;
        std::cout << "model_name        : " << UCDataset::model_name << std::endl;
        std::cout << "model_version     : " << UCDataset::model_version << std::endl;
        std::cout << "add_time          : " << UCDataset::add_time << std::endl;
        std::cout << "update_time       : " << UCDataset::update_time << std::endl;
        std::cout << "describe          : " << UCDataset::describe << std::endl;
        std::cout << "label_used        : " << UCDataset::label_used.size() <<std::endl;
        //
        if(UCDataset::label_used.size() > 0)
        {
            std::cout << "  ";
            for(int i=0; i<UCDataset::label_used.size(); i++)
            {
                std::cout << UCDataset::label_used[i];
                if(i != UCDataset::label_used.size()-1)
                {
                    std::cout << ",";
                }
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "ucd_path not exists : " << UCDataset::json_path;
    }
    std::cout << "--------------------------------" << std::endl;
}

void UCDataset::print_assign_uc_info(std::string uc)
{
    std::vector<LabelmeObj*> objects = UCDataset::object_info[uc];
    std::cout << "[" << uc << "]" << std::endl;
    for(int i=0; i<objects.size(); i++)
    {
        std::cout << objects[i]->shape_type << ", " << objects[i]->label << ", " << objects[i]->points.size() << std::endl;
    }
}

void UCDataset::unique()
{
    std::set<std::string> uc_set(UCDataset::uc_list.begin(), UCDataset::uc_list.end());
    UCDataset::uc_list.assign(uc_set.begin(), uc_set.end());
}

std::map<std::string, std::map<std::string, int> > UCDataset::count_tags()
{
    // 
    if(! is_file(UCDataset::json_path))
    {
        std::cout << "json path not exists : " << UCDataset::json_path << std::endl;
        throw "json path not exists";
    }
    // count_tags
    std::map<std::string, std::map<std::string, int> > count_map;
    UCDataset::parse_ucd(true);

    std::string each_tag;
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            std::string each_tag = iter->second[i]->label;
            std::string each_shape_type = iter->second[i]->shape_type;
            if(count_map[each_shape_type].count(each_tag) == 0)
            {
                count_map[each_shape_type][each_tag] = 1;
            }
            else
            {
                count_map[each_shape_type][each_tag] += 1;
            }
        }
        iter++;
    }
    return count_map;
}

void UCDataset::change_attar(std::string attr_name, std::string attr_value)
{
    UCDataset::parse_ucd(true);
    if(attr_name == "dataset_name")
    {
        UCDataset::dataset_name = attr_value;
    }
    else if(attr_name == "model_name")
    {
        UCDataset::model_name = attr_value;
    }
    else if(attr_name == "model_version")
    {
        UCDataset::model_version = attr_value;
    }
    else if(attr_name == "describe")
    {
        UCDataset::describe = attr_value;
    }
    else if(attr_name == "label_used")
    {
        // 将输入的 label_used 字符串根据 ',' 进行分割，获得 vector
        std::vector<std::string> label_vector = pystring::split(attr_value, ",");
        std::vector<std::string> label_used;
        for(int i=0; i<label_vector.size(); i++)
        {
            std::string label = pystring::strip(label_vector[i]);
            label_used.push_back(label);
        }
        UCDataset::label_used = label_used;
    }
    else
    {
        std::cout << "attr_name " << attr_name << "not in (dataset_name, model_name, model_version, describe, label_used)" << std::endl;
        throw "attr_name " + attr_name + " not in (dataset_name, model_name, model_version, describe, label_used)";
    }
    UCDataset::save_to_ucd(UCDataset::json_path);
}

std::vector<std::string> UCDataset::uc_slice(int start, int end)
{
    std::vector<std::string> slice;
    for(int i=start; i<end; i++)
    {
        slice.push_back(UCDataset::uc_list[i]);
    }
    return slice;
}

bool UCDataset::has_obj(std::string uc, LabelmeObj *obj)
{
    for(int j=0; j<UCDataset::object_info[uc].size(); j++)
    {
        if(UCDataset::object_info[uc][j]->equal_to(obj))
        {
            return true;
        }
    }
    return false;
}

void UCDataset::add_voc_xml_info(std::string uc, std::string voc_xml_path)
{
    // 增量解析 voc 标准的 xml 数据

    DeteRes* dete_res = new DeteRes(voc_xml_path);
    for(int i; i<dete_res->alarms.size(); i++)
    {
        DeteObj dete_obj = dete_res->alarms[i];
        RectangleObj* obj = new RectangleObj();
        obj->label = dete_obj.tag;
        obj->points = {{(double)dete_obj.x1, (double)dete_obj.y1}, {(double)dete_obj.x2, (double)dete_obj.y2}};
        if(! UCDataset::has_obj(uc, obj))
        {
            UCDataset::object_info[uc].push_back(obj);
        }
        else
        {
            std::cout << "repeated obj : " << uc << ", " << obj->label << std::endl; 
        }
    }
    
    UCDataset::uc_list.push_back(uc);
}

void UCDataset::add_labelme_json_info(std::string uc, std::string labelme_json_path)
{
    std::ifstream jsfile(labelme_json_path);
    json data = json::parse(jsfile); 

    auto shapes = data["shapes"];
    LabelmeObjFactory obj_factory;

    if(shapes == nullptr)
    { 
        std::cout << "json obj is empty" << std::endl;
    }

    for(int i=0; i<shapes.size(); i++)
    {
        std::string shape_type = shapes[i]["shape_type"];
        LabelmeObj* obj = obj_factory.CreateObj(shape_type);
        obj->points = shapes[i]["points"];
        obj->label = shapes[i]["label"];

        // 遍历查看是否已有这个对象
        if(! UCDataset::has_obj(uc, obj))
        {
            UCDataset::object_info[uc].push_back(obj);
        }
        else
        {
            std::cout << "repeated obj : " << uc << ", " << obj->label << std::endl;
        }
    }

    UCDataset::uc_list.push_back(uc);
}

void UCDataset::add_saturndatabase_json_info(std::string uc, std::string labelme_json_path)
{
    // 增量解析 labelme 标准的 json 数据

    std::ifstream jsfile(labelme_json_path);
    json data = json::parse(jsfile); 

        auto shapes = data["objects"];
        LabelmeObjFactory obj_factory;
        // std::vector<LabelmeObj*> objects;

        if(shapes == nullptr)
        { 
            std::cout << "json obj is empty" << std::endl;
        }

        for(int i=0; i<shapes.size(); i++)
        {
            std::string shape_type = shapes[i]["shape_type"];
            LabelmeObj* obj = obj_factory.CreateObj(shape_type);
            obj->points = shapes[i]["points"];
            obj->label = shapes[i]["label"];

            // 遍历查看是否已有这个对象
            if(! UCDataset::has_obj(uc, obj))
            {
                UCDataset::object_info[uc].push_back(obj);
            }
            else
            {
                std::cout << "repeated obj : " << uc << ", " << obj->label << std::endl;
            }
        }
}

void UCDataset::save_to_ucd(std::string save_path)
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
        {"shapes", {}}
    };

    std::map<std::string, std::vector<nlohmann::json> > shapes_info;
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::vector<LabelmeObj*> obj_vector = iter->second; 

        for(int i=0; i<iter->second.size(); i++)
        {
            nlohmann::json each_obj;
            each_obj["shape_type"] = obj_vector[i]->shape_type;
            each_obj["label"] = obj_vector[i]->label;
            each_obj["points"] = obj_vector[i]->points;
            shapes_info[iter->first].push_back(each_obj);
        }
        iter++;
    }    
    
    json_info["shapes"] = shapes_info;
    std::ofstream o(save_path);
    o << std::setw(4) << json_info << std::endl;
}

void UCDataset::save_to_voc_xml_with_assign_uc(std::string save_path, std::string img_path, std::string uc)
{
    int height, width, depth;
    if(! is_file(img_path))
    {
        std::cout << "img_path not exists : " << img_path << std::endl;
        height = -1;
        width = -1;
        depth = -1;
    }
    else
    {
        FILE *file = fopen(img_path.c_str(), "rb");
        auto imageInfo = getImageInfo<IIFileReader>(file);
        fclose(file);
        height = imageInfo.getHeight();
        width =  imageInfo.getWidth();
        depth = 3;
    }

    DeteRes* dete_res = new DeteRes();

    for(int i=0; i<UCDataset::object_info[uc].size(); i++)
    {
        LabelmeObj* obj = UCDataset::object_info[uc][i];
        if(obj->shape_type == "rectangle")
        {
            int x1 = obj->points[0][0];
            int y1 = obj->points[0][1];
            int x2 = obj->points[1][0];
            int y2 = obj->points[1][1];
            std::string tag = obj->label;
            dete_res->add_dete_obj(x1, y1, x2, y2, -1, tag);
        }
    }

    dete_res->width = width;
    dete_res->height = height;
    dete_res->depth = depth;
    dete_res->img_path = img_path;
    dete_res->save_to_xml(save_path);
    delete dete_res;
}

void UCDataset::save_to_labelme_json_with_assign_uc(std::string save_json_path, std::string img_path, std::string uc)
{
    // 
    cv::Mat img_mat;
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};

    nlohmann::json json_info = 
    {
        {"version", "4.4.0"},
        {"flags", {}},
        {"imagePath", ""},
        {"imageData", ""},
        {"imageHeight", -1},
        {"imageWidth", -1},
        {"shapes", {}}
    };

    if(! is_file(img_path))
    {
        std::cout << "img_path not exists : " << img_path << std::endl;
        throw "img_path not exists";
    }
    else
    {
        img_mat = cv::imread(img_path); 
        json_info["imageHeight"] = img_mat.rows;
        json_info["imageWidth"] = img_mat.cols;
    }

    std::map<std::string, nlohmann::json> obj_info;
    for(int i=0; i<UCDataset::object_info[uc].size(); i++)
    {
        LabelmeObj* obj = UCDataset::object_info[uc][i];
        obj_info["shape_type"] = obj->shape_type;
        obj_info["label"] = obj->label;
        obj_info["points"] = obj->points;
        json_info["shapes"].push_back(obj_info);
        delete obj;
    }

    std::string img_suffix = get_file_suffix(img_path);
    std::string base64_str = Mat2Base64(img_mat, img_suffix.substr(1, img_suffix.size()));
    json_info["imageData"] = base64_str;
    std::ofstream o(save_json_path);
    o << std::setw(4) << json_info << std::endl;
}

//

UCDatasetUtil::UCDatasetUtil(std::string host, int port, std::string cache_dir)
{
    UCDatasetUtil::host = host;
    UCDatasetUtil::port = port;
    UCDatasetUtil::root_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
    UCDatasetUtil::cache_dir = cache_dir;
    if(is_dir(cache_dir))
    {
        UCDatasetUtil::cache_img_dir = UCDatasetUtil::cache_dir + "/" + "img_cache";
        UCDatasetUtil::cache_xml_dir = UCDatasetUtil::cache_dir + "/" + "xml_cache";
        if(! is_dir(UCDatasetUtil::cache_img_dir))
        {
            create_folder(UCDatasetUtil::cache_img_dir);
        }
        if(! is_dir(UCDatasetUtil::cache_xml_dir))
        {
            create_folder(UCDatasetUtil::cache_xml_dir);
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
    ucd->parse_ucd();

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

        if(res == nullptr)
        {
            std::cout << "connect error : " << url << std::endl;
            return;
        }

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
        }
    }
}

void UCDatasetUtil::load_img(std::string save_dir, std::vector<std::string> uc_list)
{
    if(! is_dir(save_dir))
    {
        std::cout << "save dir not exists : " << save_dir << std::endl;
        throw "save dir not exists";
    }

    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};

    for(int i=0; i<uc_list.size(); i++)
    {
        std::string img_url = "/file/" + uc_list[i] + ".jpg";
        std::string save_img_path = save_dir + "/" + uc_list[i] + ".jpg";  
                
        if(! is_file(save_img_path))
        {
            std::string img_cache_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc_list[i], suffix);
            if(is_file(img_cache_path))
            {
                copy_file(img_cache_path, save_img_path);
                std::cout << i << " " << "* copy " << img_cache_path << std::endl;
            }
            else
            {
                UCDatasetUtil::load_file(img_url, save_img_path, i); 
            }
        }
        else
        {
            std::cout << i << ", file exists : " << save_img_path << std::endl;
        }
    }
}

void UCDatasetUtil::load_xml(std::string save_dir, std::vector<std::string> uc_list)
{
    if(! is_dir(save_dir))
    {
        std::cout << "save dir not exists : " << save_dir << std::endl;
        throw "save dir not exists";
    }

    for(int i=0; i<uc_list.size(); i++)
    {
        std::string xml_url = "/file/" + uc_list[i] + ".xml";
        std::string save_xml_path = save_dir + "/" + uc_list[i] + ".xml";  
        if(! is_file(save_xml_path))
        {
            UCDatasetUtil::load_file(xml_url, save_xml_path, i); 
        }
        else
        {
            std::cout << i <<  ", file exists : " << save_xml_path << std::endl;
        }
    }
}

void UCDatasetUtil::load_ucd(std::string ucd_name, std::string save_path)
{
    UCDatasetUtil::load_file("/ucd/" + ucd_name, save_path);
}


// 
void UCDatasetUtil::search_ucd()
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

    UCDataset* ucd = new UCDataset(ucd_path);

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
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_xml_dir(std::string xml_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".xml"};
    std::vector<std::string> xml_path_vector = get_all_file_path_recursive(xml_dir, suffix);
    UCDataset* ucd = new UCDataset(ucd_path);
    std::string uc;

    for(int i=0; i<xml_path_vector.size(); i++)
    {
        uc = get_file_name(xml_path_vector[i]);
        if(is_uc(uc))
        {
            std::cout << i << ", add labelme json : " << xml_path_vector[i] << std::endl;
            ucd->add_voc_xml_info(uc, xml_path_vector[i]);
        }
    }
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_json_dir(std::string json_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".json"};
    std::vector<std::string> json_path_vector = get_all_file_path_recursive(json_dir, suffix);

    UCDataset* ucd = new UCDataset(ucd_path);
    std::string uc, json_path;

    for(int i=0; i<json_path_vector.size(); i++)
    {
        uc = get_file_name(json_path_vector[i]);
        if(is_uc(uc))
        {
            std::cout << i << ", add labelme json : " << json_path_vector[i] << std::endl;
            ucd->add_labelme_json_info(uc, json_path_vector[i]);
        }
    }
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector)
{

    // todo object_info 也需要进行合并


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
    UCDataset* merge = new UCDataset(save_path);
    std::set<std::string> uc_set, used_label_set;
    for(int i=0; i<ucd_path_vector.size(); i++)
    {
        UCDataset* ucd = new UCDataset(ucd_path_vector[i]);
        ucd->parse_ucd();
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
    merge->save_to_ucd(save_path);
    delete merge;
}

void UCDatasetUtil::ucd_diff(std::string ucd_path_1, std::string ucd_path_2)
{
    UCDataset* ucd1 = new UCDataset(ucd_path_1);
    ucd1->parse_ucd();
    UCDataset* ucd2 = new UCDataset(ucd_path_2);
    ucd2->parse_ucd();
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
    ucd1->parse_ucd();
    UCDataset* ucd2 = new UCDataset(ucd_path_2);
    ucd2->parse_ucd();
    UCDataset* ucd_res = new UCDataset(save_path);
    std::vector<std::string> uc_difference;
    // set_intersection
    std::set<std::string> uc_set1(ucd1->uc_list.begin(), ucd1->uc_list.end());
    std::set<std::string> uc_set2(ucd2->uc_list.begin(), ucd2->uc_list.end());
    std::set_difference(uc_set1.begin(), uc_set1.end(), uc_set2.begin(), uc_set2.end(), std::inserter(uc_difference, uc_difference.begin()));
    // save
    ucd_res->uc_list = uc_difference;
    ucd_res->save_to_ucd(save_path);
    // delete
    delete ucd1;
    delete ucd2;
    delete ucd_res;
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

    int tag_count = 0;
    int dete_obj_count=0; 
    UCDataset* ucd = new UCDataset(ucd_path);
    // count_tags 函数中会自动 解析一遍 json 
    std::map<std::string, std::map<std::string, int> > count_map = ucd->count_tags();

    // print statistics res
    auto iter_count = count_map.begin();
    std::cout << "---------------------------------------------" << std::endl;
    while(iter_count != count_map.end())
    {
        auto iter = iter_count->second.begin();
        while(iter != iter_count->second.end())
        {
            dete_obj_count += iter->second;
            std::cout << std::setw(15) << std::left << "[" + iter_count->first + "]" << std::setw(15) << std::left << iter->first  << " : " << iter->second << std::endl;
            tag_count += 1;
            iter++;
        }
        iter_count ++;
    }
    std::cout << "---------------------------" << std::endl;
    std::cout << "number of uc  : " << ucd->uc_list.size() << std::endl;
    std::cout << "number of tag : " << tag_count << std::endl;
    std::cout << "number of obj : " << dete_obj_count << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    delete ucd;
}

void UCDatasetUtil::cache_clear()
{

    if(! is_dir(UCDatasetUtil::cache_img_dir))
    {
        std::cout << "ucd path not exists" << std::endl;
    }

    // 清空全部缓存
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> all_img_path_vector = get_all_file_path(UCDatasetUtil::cache_img_dir, suffix);
    for(int i=0; i<all_img_path_vector.size(); i++)
    {
        remove(all_img_path_vector[i].c_str());
        std::cout << i << " , remove : " << all_img_path_vector[i] << std::endl;
    }
}

void UCDatasetUtil::cache_clear(std::string ucd_path)
{
    if(! is_dir(UCDatasetUtil::cache_img_dir))
    {
        std::cout << "ucd path not exists" << std::endl;
    }

    // 清空全部缓存
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd();

    for(int i=0; i<ucd->uc_list.size(); i++)
    {
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, ucd->uc_list[i], suffix);

        if(is_file(img_path))
        {
            remove(img_path.c_str());
            std::cout << i << " , remove : " << img_path << std::endl;
        }
    }
    delete ucd;
}

void UCDatasetUtil::print_words(std::string name, int width, int height)
{
    // refer : https://www.codeleading.com/article/25882730552/
    std::string strChar;
    std::vector<std::string> words;
    for(int i = 0; name[i] != '\0'; )
    {
        char chr = name[i];
        if((chr & 0x80) == 0)
        {
            strChar = name.substr(i,1);
            words.push_back(strChar);
            ++i;
        }
        else if((chr & 0xF8) == 0xF8)
        {
            strChar = name.substr(i, 5);
            words.push_back(strChar);
            i+=5;
        }
        else if((chr & 0xF0) == 0xF0)
        {
            strChar = name.substr(i, 4);
            words.push_back(strChar);
            i+=4;
        }
        else if((chr & 0xE0) == 0xE0)
        {
            strChar = name.substr(i, 3);
            words.push_back(strChar);
            i+=3;
        }
        else if((chr & 0xC0) == 0xC0)
        {
            strChar = name.substr(i, 2);
            words.push_back(strChar);
            i+=2;
        }
    }
    
    
    std::string save_dir = UCDatasetUtil::cache_dir + "/" + "word";
    if(! is_dir(save_dir))
    {
        std::cout << "word folder not exists : " << save_dir << std::endl;
        return;
    }

    // get word mat
    std::vector<cv::Mat> word_mat_vector;
    for(int i=0; i<words.size(); i++)
    {
        std::string word_path = save_dir + "/" + words[i] + ".png";
        std::cout << word_path << std::endl;
        if(is_file(word_path))
        {
            cv::Mat word_mat = cv::imread(word_path);
            cv::Mat word_resize;
            cv::resize(word_mat, word_resize, cv::Size(width, height));
            word_mat_vector.push_back(word_resize);
        }
        else
        {
            std::cout << "* 未找到字符 : " << words[i] << "，目前仅支持中文"<< std::endl;
            throw "未找到字符 : " + words[i] ;
        }
    }

    // print
    for(int i=0; i<height; i++)
    {
        for(int m=0; m<word_mat_vector.size(); m++)
        {
            for(int j=0; j<width; j++)
            {
                int index = (i * width + j)*3;
                int pix_value = (int)word_mat_vector[m].data[index];
                if(pix_value > 254)
                {
                    std::cout << "  ";
                }
                else
                {
                    std::cout << "**";
                }
            }
        }
        std::cout << std::endl;
    }
}

void UCDatasetUtil::cut_small_img(std::string img_dir, std::string xml_dir, std::string save_dir, bool is_split)
{
    // // 
    // if(! is_dir(save_dir))
    // {
    //     std::cout << "save dir not exists : " << save_dir << std::endl;
    //     throw "save dir not exists";
    // }

    // UCDataset* ucd; 
    // // 
    // if(UCDatasetUtil::is_ucd_path(xml_dir))
    // {
    //     ucd = new UCDataset(xml_dir);
    //     ucd->parse_ucd(true);
    //     if(ucd->xml_info.size() == 0)
    //     {
    //         std::cout << "no dete_obj in ucd_path : " << xml_dir << std::endl;
    //         throw "no dete_obj in ucd_path";
    //     }
    // }
    // else if(is_dir(xml_dir))
    // {
    //     // 非 ucd 先转为 ucd 结构
    //     ucd = new UCDataset("");
    //     std::set<std::string> suffix {".xml"};
    //     std::vector<std::string> xml_path_vector = get_all_file_path(xml_dir, suffix);
    //     for(int i=0; i<xml_path_vector.size(); i++)
    //     {
    //         std::string uc = get_file_name(xml_path_vector[i]);
    //         if(is_uc(uc))
    //         {
    //             ucd->uc_list.push_back(uc);
    //             DeteRes* dete_res = new DeteRes(xml_path_vector[i]);
    //             std::vector< std::vector<std::string> > each_xml_info;
    //             for(int j=0; j<dete_res->alarms.size(); j++)
    //             {
    //                 std::vector<std::string> dete_obj_info;
    //                 dete_obj_info.push_back(std::to_string(dete_res->alarms[j].x1));        
    //                 dete_obj_info.push_back(std::to_string(dete_res->alarms[j].y1));        
    //                 dete_obj_info.push_back(std::to_string(dete_res->alarms[j].x2));        
    //                 dete_obj_info.push_back(std::to_string(dete_res->alarms[j].y2));        
    //                 dete_obj_info.push_back(std::to_string(dete_res->alarms[j].conf));        
    //                 dete_obj_info.push_back(dete_res->alarms[j].tag);
    //                 each_xml_info.push_back(dete_obj_info);
    //             }
    //             ucd->xml_info[uc] = each_xml_info;
    //             delete dete_res;
    //         }
    //     }
    // }
    // else
    // {
    //     std::cout << "xml_dir need xml_dir or ucd_path : " << xml_dir << std::endl;
    //     throw "xml_dir need xml_dir or ucd_path";
    // }

    // //
    // if(UCDatasetUtil::is_ucd_path(img_dir) && (! is_dir(UCDatasetUtil::cache_img_dir)))
    // {
    //     std::cout << "if img_dir is ucd_path, need set cache_dir in ucconfig : " << img_dir << std::endl;
    //     throw "if img_dir is ucd_path, need set cache_dir in ucconfig";
    // }
    // else if(UCDatasetUtil::is_ucd_path(img_dir) || is_dir(img_dir))
    // {
    //     // 图像如果是 ucd 先下载在进行处理
    //     std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    //     for(int i=0; i<ucd->uc_list.size(); i++)
    //     {
    //         std::string uc = ucd->uc_list[i];
    //         // 查看是否有对应的 dete_obj 如果没有跳过
    //         if(ucd->xml_info[uc].size() == 0)
    //         {
    //             continue;
    //         }

    //         std::string img_path;
    //         if(UCDatasetUtil::is_ucd_path(img_dir))
    //         {
    //             // 文件不存在就先下载文件 
    //             img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, ucd->uc_list[i], img_suffix);
    //             if(! is_file(img_path))
    //             {
    //                 std::string img_url = "/file/" + ucd->uc_list[i] + ".jpg";
    //                 std::string save_img_path = UCDatasetUtil::cache_img_dir + "/" + ucd->uc_list[i] + ".jpg";
    //                 UCDatasetUtil::load_file(img_url, save_img_path, -1);
    //                 //
    //                 if(is_file(save_img_path))
    //                 {
    //                     img_path = save_img_path;
    //                 }
    //                 else
    //                 {
    //                     std::cout << "load uc img failed : " << ucd->uc_list[i] << std::endl;
    //                     continue;
    //                 }
    //             }
    //         }
    //         else
    //         {
    //             // 找到对应的图片，找不到就跳过
    //             img_path = get_file_by_suffix_set(img_dir, uc, img_suffix);
    //             if(! is_file(img_path))
    //             {
    //                 continue;
    //             }
    //         }

    //         // 图片下载完毕，等待裁切
    //         DeteRes* dete_res = new DeteRes();
    //         dete_res->img_path = img_path;
    //         for(int j=0; j<ucd->xml_info[uc].size(); j++)
    //         {
    //             // x1, y1, x2, y2, conf, tag
    //             std::vector<std::string> dete_obj_info = ucd->xml_info[uc][j];
    //             DeteObj* dete_obj = new DeteObj();
    //             dete_obj->x1 = std::stoi(dete_obj_info[0]);
    //             dete_obj->y1 = std::stoi(dete_obj_info[1]);
    //             dete_obj->x2 = std::stoi(dete_obj_info[2]);
    //             dete_obj->y2 = std::stoi(dete_obj_info[3]);
    //             dete_obj->conf = std::stof(dete_obj_info[4]);
    //             dete_obj->tag = dete_obj_info[5];
    //             dete_res->add_dete_obj(*dete_obj);
    //         }
    //         std::cout << i << ", cut uc : " << uc << std::endl;
    //         dete_res->parse_img_info(img_path);
    //         dete_res->crop_dete_obj(save_dir, is_split);
    //     }
    // }
    // else
    // {
    //     std::cout << "img_dir is not exists : " << img_dir << std::endl;
    //     throw "img_dir is not exists";
    // }
    // delete ucd;
}

void UCDatasetUtil::parse_labelme_json(std::string img_dir, std::string save_dir, std::string ucd_path)
{
    if(! is_dir(img_dir))
    {
        std::cout << "img_dir not exists : " << img_dir << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << "save_dir not exists : " << save_dir << std::endl;
        throw "save_dir not exists";
    }

    if(! is_file(ucd_path))
    {
        std::cout << "ucd_path not exists : " << ucd_path << std::endl;
        throw "ucd_path not exists";
    }

    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    int index = 0;
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        UCDatasetUtil::load_img(UCDatasetUtil::cache_img_dir, {uc});
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        
        if(is_file(img_path))
        {
            std::string json_path = save_dir + "/" + uc + ".json";
            ucd->save_to_labelme_json_with_assign_uc(json_path, img_path, uc);
            std::cout << index << ", parse json : " << uc << std::endl;
        }
        else
        {
            std::cout << "load img failed : " << img_path << std::endl;
        }
        index += 1;
        iter ++;
    }
}

void UCDatasetUtil::parse_voc_xml(std::string img_dir, std::string save_dir, std::string ucd_path)
{
    if(! is_dir(img_dir))
    {
        std::cout << "img_dir not exists : " << img_dir << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << "save_dir not exists : " << save_dir << std::endl;
        throw "save_dir not exists";
    }

    if(! is_file(ucd_path))
    {
        std::cout << "ucd_path not exists : " << ucd_path << std::endl;
        throw "ucd_path not exists";
    }

    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    int index = 0;
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        UCDatasetUtil::load_img(UCDatasetUtil::cache_img_dir, {uc});
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        
        if(is_file(img_path))
        {
            std::string xml_path = save_dir + "/" + uc + ".xml";
            ucd->save_to_voc_xml_with_assign_uc(xml_path, img_path, uc);
            std::cout << index << ", parse xml : " << uc << std::endl;
        }
        else
        {
            std::cout << "load img failed : " << img_path << std::endl;
        }
        index += 1;
        iter ++;
    }
}


// void UCDatasetUtil::uc_check(std::vector<std::string> file_vector)
// {

//     std::vector<std::string> file_vector = get_all_file_path(file_dir);

//     SaturnDatabaseSQL *sd_sql = new SaturnDatabaseSQL(sql_host, sql_port, sql_user, sql_pwd, sql_db);

//     std::vector<std::string> uc_vector {uc};
//     std::map<std::string, bool> is_uc_map = sd_sql->check_uc_by_mysql(uc_vector);



// }
