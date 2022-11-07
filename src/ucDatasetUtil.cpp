

#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <set>
#include <iterator>
#include "include/fileOperateUtil.hpp"
#include "include/ucDatasetUtil.hpp"
#include "include/deteRes.hpp"
#include "include/pystring.h"
#include "include/strToImg.hpp"
#include "include/lablelmeObj.hpp"
#include "include/easyexif.h"
#include "include/imageinfo.hpp"
#include "include/tqdm.h"
#include <cstdlib>

using json = nlohmann::json;
using namespace jotools;

#define ERROR_COLOR         "\x1b[35m"
#define WARNNING_COLOR      "\033[33m"
#define STOP_COLOR          "\033[0m"



// CPP 写服务端 refer : https://blog.csdn.net/canlynetsky/article/details/119083255

// 自己写的服务，上传下载大文件出错的问题，

static bool is_uc(std::string uc)
{

    if(uc.size() != 7){ return false; }

    if(((int)uc[0] < (int)'C') || ((int)uc[0] > int('K'))) { return false; }
    if(((int)uc[1] < (int)'a') || ((int)uc[1] > int('z'))) { return false; }
    if(((int)uc[2] < (int)'a') || ((int)uc[2] > int('z'))) { return false; }
    return true;
}

static std::map< std::string, Color > read_color_map(std::string color_file_path)
{
    if(! is_file(color_file_path))
    {
        std::cout << ERROR_COLOR << "color file path not exists : " << color_file_path << STOP_COLOR << std::endl;
        throw "color file path not exists";
    }

    std::ifstream read_file;
    read_file.open(color_file_path);
    assert(read_file.is_open());
    std::map< std::string, Color > color_map;

    // 过滤掉开头为 # 的行
    std::string line;
    while(getline(read_file, line))
    {
        line = pystring::strip(line);
        if(line.size() == 0)
        {
            continue;
        }
        else if(line[0] == '#')
        {
            continue;
        }
        else
        {   
            Color each_color;
            std::string tag;
            std::vector<std::string> split_res = pystring::split(line, ",");
            tag = split_res[0];
            each_color.r    = std::stoi(split_res[1]); 
            each_color.g    = std::stoi(split_res[2]); 
            each_color.b    = std::stoi(split_res[3]);
            color_map[tag]  = each_color; 
        }
    }
    read_file.close();
    return color_map;
}

static void write_color_map(std::map< std::string, Color > color_map, std::string color_file_path)
{
    std::ofstream OutFile(color_file_path); 
    if(color_map.count("default") > 0)
    {
        Color color = color_map["default"];
        OutFile << "default" << "," << color.r << "," << color.g << "," << color.b << std::endl;
        OutFile << std::endl;
    }

    auto iter = color_map.begin();
    while(iter != color_map.end())
    {
        if(iter->first != "default")
        {
            OutFile << iter->first << "," << iter->second.r << "," << iter->second.g << "," << iter->second.b << std::endl;
        }
        iter++;
    }
    OutFile.close();
}

UCDataset::UCDataset(std::string json_path)
{
    UCDataset::dataset_name     = "";
    UCDataset::model_name       = "";
    UCDataset::model_version    = "";
    UCDataset::add_time         = -1;
    UCDataset::update_time      = -1;
    UCDataset::describe         = "";
    UCDataset::json_path        = json_path;
    UCDataset::volume_count     = 0;
    UCDataset::volume_size      = -1;
}

// UCDataset::~UCDataset()
// 使用析构函数之后 delete ucd 会报错，不知道原因
// {
//     // 删除 obj 信息
//     UCDataset::clear_obj_info();
//     // 
//     UCDataset::object_info.clear();
//     UCDataset::size_info.clear();
//     UCDataset::uc_list.clear();
// }

void UCDataset::parse_ucd(bool parse_shape_info)
{
    if(! is_file(UCDataset::json_path))
    {
        std::cout << "json path not exists : " << UCDataset::json_path << std::endl;
        throw "json path not exists";
    }

    std::ifstream jsfile(UCDataset::json_path);
    json data = json::parse(jsfile); 

    auto dataset_name   = data["dataset_name"];
    auto model_name     = data["model_name"];
    auto model_version  = data["model_version"];
    auto add_time       = data["add_time"];
    auto update_time    = data["update_time"];
    auto describe       = data["describe"];
    auto label_used     = data["label_used"];
    auto uc_list        = data["uc_list"];
    auto size_info      = data["size_info"];

    if(dataset_name != nullptr){ UCDataset::dataset_name = dataset_name; }
    if(model_name != nullptr){ UCDataset::model_name = model_name; }
    if(model_version != nullptr){ UCDataset::model_version = model_version; }
    if(add_time != nullptr){ UCDataset::add_time = add_time; }
    if(update_time != nullptr){ UCDataset::update_time = update_time; }
    if(describe != nullptr){ UCDataset::describe = describe; }
    if(label_used != nullptr){ UCDataset::label_used = label_used; }
    if(uc_list != nullptr){ UCDataset::uc_list = uc_list; }
    if(size_info != nullptr){ UCDataset::size_info = size_info; }
    UCDataset::unique();

    // parse shape_info
    if(parse_shape_info)
    {
        auto shapes_info = data["shapes"];
        LabelmeObjFactory obj_factory;

        if(shapes_info != nullptr)
        {
            tqdm bar;
            int N = shapes_info.size();
            int index = 0;
            auto iter = shapes_info.begin();
            while(iter != shapes_info.end())
            {
                index += 1;
                bar.progress(index, N);
                std::string uc = iter.key();
                for(int i=0; i<iter.value().size(); i++)
                {
                    std::string shape_type = iter.value()[i]["shape_type"];
                    std::string label = iter.value()[i]["label"];
                    std::vector< std::vector<double> > points = iter.value()[i]["points"];
                    auto conf = iter.value()[i]["conf"];
                    LabelmeObj* obj = obj_factory.CreateObj(shape_type);
                    obj->label = label;
                    obj->points = points;

                    if(conf == nullptr)
                    {
                        obj->conf = -1;                      
                    }
                    else
                    {
                        obj->conf = conf;
                    }

                    UCDataset::object_info[uc].push_back(obj);
                }
                iter++;
            }
            bar.finish();
        }
    }
}

void UCDataset::print_ucd_info()
{
    // print statistics res
    if(is_file(UCDataset::json_path))
    {
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
        std::cout << "img_size_count    : " << UCDataset::size_info.size() <<std::endl;
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
        std::cout << "ucd_path not exists : " << UCDataset::json_path << std::endl;
        return;
    }
    std::cout << "--------------------------------" << std::endl;
}

void UCDataset::print_volume_info()
{
    // UCDataset* ucd = new UCDataset();
    // UCDataset::load_uci(UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".uci");

    tqdm bar;
    int N = UCDataset::volume_count;
    int img_size_count = 0;
    int uc_count = 0;

    for(int i=0; i<UCDataset::volume_count; i++)
    {
        bar.progress(i, N);
        UCDataset::parse_volume(i, false);
        img_size_count += UCDataset::size_info.size();
        uc_count += UCDataset::uc_list.size();
    }
    bar.finish();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "dataset_name      : " << UCDataset::dataset_name << std::endl;
    std::cout << "uc_count          : " << uc_count << std::endl;
    std::cout << "model_name        : " << UCDataset::model_name << std::endl;
    std::cout << "model_version     : " << UCDataset::model_version << std::endl;
    std::cout << "add_time          : " << UCDataset::add_time << std::endl;
    std::cout << "update_time       : " << UCDataset::update_time << std::endl;
    std::cout << "describe          : " << UCDataset::describe << std::endl;
    std::cout << "label_used        : " << UCDataset::label_used.size() <<std::endl;
    // std::cout << "img_size_count    : " << img_size_count <<std::endl;

    // label used
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
    std::cout << "--------------------------------" << std::endl;
    std::cout << "volume_size       : " << UCDataset::volume_size <<std::endl;
    std::cout << "volume count      : " << UCDataset::volume_count << std::endl;
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

    std::set<std::string> label_set(UCDataset::label_used.begin(), UCDataset::label_used.end());
    UCDataset::label_used.assign(label_set.begin(), label_set.end());
}

std::map<std::string, std::map<std::string, int> > UCDataset::count_tags()
{
    // 统计速度太快了，不用进度条
    if(! is_file(UCDataset::json_path))
    {
        std::cout << "json path not exists : " << UCDataset::json_path << std::endl;
        throw "json path not exists";
    }
    // count_tags
    std::map<std::string, std::map<std::string, int> > count_map;

    // std::string each_tag;
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

std::map<std::string, std::map<std::string, int> > UCDataset::count_volume_tags()
{
    std::string uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".uci";
    UCDataset::load_uci(uci_path);

    // count_tags
    std::map<std::string, std::map<std::string, int> > count_map;

    tqdm bar;
    int N = UCDataset::volume_count;
    
    for(int i=0; i<UCDataset::volume_count; i++)
    {
        bar.progress(i, N);
        UCDataset::parse_volume(i, false, true);
        
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
    }
    bar.finish();
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

void UCDataset::delete_obj(std::string uc, LabelmeObj *obj)
{
    for(int j=0; j<UCDataset::object_info[uc].size(); j++)
    {
        if(UCDataset::object_info[uc][j]->equal_to(obj))
        {
            UCDataset::object_info[uc].erase(UCDataset::object_info[uc].begin() + j);
        }
    }
}

void UCDataset::add_obj(std::string uc, LabelmeObj *obj)
{
    if(! UCDataset::has_obj(uc, obj))
    {
        UCDataset::object_info[uc].push_back(obj);
    }
}

void UCDataset::add_voc_xml_info(std::string uc, std::string voc_xml_path)
{
    // 增量解析 voc 标准的 xml 数据

    DeteRes* dete_res = new DeteRes(voc_xml_path);
    
    // img_size
    int width = dete_res->width;
    int height = dete_res->height;
    if(width > 0 && height > 0)
    {
        UCDataset::size_info[uc] = {width, height};
    }

    // img_obj
    for(int i; i<dete_res->alarms.size(); i++)
    {
        DeteObj dete_obj = dete_res->alarms[i];
        RectangleObj* obj = new RectangleObj();
        obj->label = dete_obj.tag;
        obj->conf = dete_obj.conf;
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

    // // 尝试彻底删除数据
    // for(int i=0; i<UCDataset::object_info[uc].size(); i++)
    // {
    //     delete UCDataset::object_info[uc][i];
    //     // std::cout << UCDataset::object_info[uc][i]->label << std::endl;
    // }

    // uc_list 
    UCDataset::uc_list.push_back(uc);
    delete dete_res;
}

void UCDataset::add_labelme_json_info(std::string uc, std::string labelme_json_path)
{
    std::ifstream jsfile(labelme_json_path);
    json data = json::parse(jsfile); 

    auto shapes = data["shapes"];
    LabelmeObjFactory obj_factory;

    // size 
    int height = data["imageHeight"];
    int width = data["imageWidth"];
    if(width > 0 && height > 0)
    {
        UCDataset::size_info[uc] = {width, height};
    }

    // obj info
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
        obj->conf = -1;
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
            obj->conf = -1;
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

void UCDataset::add_dete_res_info(std::string uc, DeteRes dete_res)
{
    for(int i; i<dete_res.alarms.size(); i++)
    {
        DeteObj dete_obj = dete_res.alarms[i];
        RectangleObj* obj = new RectangleObj();
        obj->label = dete_obj.tag;
        obj->conf = dete_obj.conf;
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

void UCDataset::add_ucd_info(std::string ucd_path)
{
    UCDataset * other = new UCDataset(ucd_path);
    other->parse_ucd(true);
    UCDataset::add_ucd_info(other);
}

void UCDataset::add_ucd_info(UCDataset* other)
{
    // merge uc_list
    for(int i=0; i<other->uc_list.size(); i++)
    {
        UCDataset::uc_list.push_back(other->uc_list[i]);
    }

    // merge label_used
    for(int i=0; i<other->label_used.size(); i++)
    {
        UCDataset::label_used.push_back(other->label_used[i]);
    }

    UCDataset::unique();

    // merge size_info
    auto iter_size = other->size_info.begin();
    while(iter_size != other->size_info.end())
    {
        UCDataset::size_info[iter_size->first] = iter_size->second;
        iter_size++;
    }

    // merge object_info 
    auto iter = other->object_info.begin();
    while(iter != other->object_info.end())
    {
        std::string uc = iter->first;
        if(UCDataset::object_info.count(uc) == 0)
        {
            UCDataset::object_info[uc] = other->object_info[uc]; 
        }
        else
        {
            for(int j=0; j<iter->second.size(); j++)
            {
                auto obj = iter->second[j];
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
        iter++;
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
        {"shapes", {}},
        {"size_info", UCDataset::size_info}
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
            each_obj["conf"] = obj_vector[i]->conf;
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
    height = -1;
    width = -1;
    depth = -1;
    
    if(UCDataset::size_info.count(uc) != 0)
    {
        std::vector<int> size_info = UCDataset::size_info[uc];
        if(size_info.size() == 2)
        {
            width = size_info[0];
            height = size_info[1];
            depth = 3;
        }
    }

    // 
    if((height < 1) || (width < 1))
    {
        if(! is_file(img_path))
        {
            std::cout << "img_path not exist and size_info is empty, can't get img_size, ignore: " << uc << std::endl;
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
    }

    DeteRes* dete_res = new DeteRes();

    if(UCDataset::object_info.count(uc) != 0)
    {
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
            float conf = obj->conf;
            dete_res->add_dete_obj(x1, y1, x2, y2, conf, tag);
        }
    }

    dete_res->width = width;
    dete_res->height = height;
    dete_res->depth = depth;
    dete_res->img_path = img_path;
    dete_res->save_to_xml(save_path);
    }
    delete dete_res;
}

void UCDataset::save_to_yolo_train_txt_with_assign_uc(std::string save_path, std::string img_path, std::string uc, std::vector<std::string> label_list)
{
    std::map<std::string, int> label_map;
    for(int i=0; i<label_list.size(); i++)
    {
        label_map[label_list[i]] = i;
    }

    int height, width, depth;
    height = -1;
    width = -1;
    depth = -1;
    
    if(UCDataset::size_info.count(uc) != 0)
    {
        std::vector<int> size_info = UCDataset::size_info[uc];
        if(size_info.size() == 2)
        {
            width = size_info[0];
            height = size_info[1];
        }
    }

    // 
    if((height < 1) || (width < 1))
    {
        if(! is_file(img_path))
        {
            std::cout << "size_info is empty and img_path not exists, ignore this uc : " << uc << std::endl;
            return;
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
    }

    std::vector< std::vector<std::string> > txt_info;
    float dh = 1.0 / height;
    float dw = 1.0 / width;

    if(UCDataset::object_info.count(uc) != 0)
    {
        for(int i=0; i<UCDataset::object_info[uc].size(); i++)
        {
            LabelmeObj* obj = UCDataset::object_info[uc][i];
            std::vector<std::string> each_txt_info;
            if(obj->shape_type == "rectangle")
            {
                float x1 = obj->points[0][0];
                float y1 = obj->points[0][1];
                float x2 = obj->points[1][0];
                float y2 = obj->points[1][1];
                std::string tag = obj->label;

                if(label_map.count(tag) == 0)
                {
                    std::cout << "tag not in tag_list, " << tag << std::endl;
                    continue;
                }
                else
                {
                    float x = ((x1 + x2) / 2.0 - 1) * dw;
                    float y = ((y1 + y2) / 2.0 - 1) * dh;
                    float w = (x2 - x1) * dw;
                    float h = (y2 - y1) * dh;

                    if((x <= 1) && (y <= 1) && (w <= 1) && (h <= 1))
                    {
                        int label_index = label_map[tag];
                        each_txt_info.push_back(std::to_string(label_index));   
                        each_txt_info.push_back(std::to_string(x));   
                        each_txt_info.push_back(std::to_string(y));   
                        each_txt_info.push_back(std::to_string(w));   
                        each_txt_info.push_back(std::to_string(h));
                        txt_info.push_back(each_txt_info);
                    }
                    else
                    {
                        std::cout << "x, y, w, h, not in range [0, 1]" << std::endl;
                        continue;
                    }
                }
            }
        }

        // save to txt
        std::ofstream ofs;
        ofs.open(save_path);
        if (!ofs.is_open()) 
        {
            std::cout << "Failed to open file : " << save_path << std::endl;
        } 
        else 
        {
            for(int i=0; i<txt_info.size(); i++)
            {
                ofs << txt_info[i][0] << " " << txt_info[i][1] << " " << txt_info[i][2] << " " << txt_info[i][3] << " " << txt_info[i][4] << "\n";
            }
            ofs.close();
        }
    }
}

void UCDataset::get_dete_res_with_assign_uc(jotools::DeteRes *dete_res, std::string uc)
{
    if(UCDataset::object_info.count(uc) != 0)
    {
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
                float conf = obj->conf;
                dete_res->add_dete_obj(x1, y1, x2, y2, conf, tag);
            }
        }
    }
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
    
    if(UCDataset::object_info.count(uc) != 0)
    {
        for(int i=0; i<UCDataset::object_info[uc].size(); i++)
        {
            LabelmeObj* obj = UCDataset::object_info[uc][i];
            obj_info["shape_type"] = obj->shape_type;
            obj_info["label"] = obj->label;
            obj_info["points"] = obj->points;
            json_info["shapes"].push_back(obj_info);
            delete obj;
        }
    }

    std::string img_suffix = get_file_suffix(img_path);
    std::string base64_str = Mat2Base64(img_mat, img_suffix.substr(1, img_suffix.size()));
    json_info["imageData"] = base64_str;
    std::ofstream o(save_json_path);
    o << std::setw(4) << json_info << std::endl;
}

void UCDataset::filter_by_conf(float conf_th, bool clear_obj)
{
    // todo 是不是有垃圾需要删掉，如何删除
    std::vector<DeteObj> new_alarms;
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::vector<LabelmeObj*> objs = iter->second;
        for(auto iter_o = objs.begin(); iter_o != objs.end();)
        {
            if((*iter_o)->conf < conf_th)
            {
                iter_o = objs.erase(iter_o);
            }
            else
            {
                iter_o++;
            }
        }
        iter->second = objs;
        iter++;
    }
}

void UCDataset::filter_by_nms(float nms_th, bool ignore_tag, bool clear_obj)
{
    // 先用比较搓的办法，后续要进行重写

    // 释放已经不使用的资源

    UCDataset* ucd = new UCDataset();

    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::string uc = iter->first;
        DeteRes* dete_res = new DeteRes();
        UCDataset::get_dete_res_with_assign_uc(dete_res, uc);
        dete_res->do_nms(nms_th, ignore_tag);
        ucd->add_dete_res_info(uc, *dete_res);
        iter++;
    }
    UCDataset::object_info = ucd->object_info;
}

void UCDataset::filter_by_tags(std::set<std::string> tags, bool clear_obj)
{
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::vector<LabelmeObj*> objs;
        for(int i=0; i<iter->second.size(); i++)
        {
            LabelmeObj* obj = iter->second[i];
            if(tags.count(obj->label) > 0)
            {
                objs.push_back(obj);
            }
            else if(clear_obj)
            {
                delete obj;
            }
        }
        iter->second = objs;
        if(objs.size() == 0)
        {
            UCDataset::size_info.erase(iter->first);
        }
        iter++;
    }
}

void UCDataset::crop_dete_res_with_assign_uc(std::string uc, std::string img_path, std::string save_dir)
{
    if(UCDataset::object_info.count(uc) == 0)
    {
        std::cout << "uc not in object_info : " << uc << std::endl;
        return ;
    }

    if(! is_file(img_path))
    {
        std::cout << "img_path not exists" << std::endl;
        return;
    }

    jotools::DeteRes* dete_res = new DeteRes();
    dete_res->parse_img_info(img_path);
    UCDataset::get_dete_res_with_assign_uc(dete_res, uc);
    dete_res->crop_dete_obj(save_dir, true);
    delete dete_res;
}

void UCDataset::get_sub_ucd(int sub_count, bool is_random, std::string save_path)
{
    if(sub_count > UCDataset::uc_list.size())
    {
        std::cout << "sub_count > uc_list.size()" << std::endl;
        throw "sub_count > uc_list.size()";
    }

    UCDataset* new_ucd = new UCDataset(save_path);
    new_ucd->dataset_name   = UCDataset::dataset_name;
    new_ucd->model_version  = UCDataset::model_version;
    new_ucd->model_name     = UCDataset::model_name;
    new_ucd->describe       = UCDataset::describe;
    new_ucd->add_time       = UCDataset::add_time;
    new_ucd->update_time    = -1;
    new_ucd->label_used     = UCDataset::label_used;
    
    // get new uc_list
    std::vector<std::string> uc_list = UCDataset::uc_list;
    if(is_random)
    {
        std::random_shuffle(uc_list.begin(), uc_list.end());
    }
    
    // get new info
    for(int i=0; i<sub_count; i++)
    {
        std::string uc = uc_list[i];
        new_ucd->uc_list.push_back(uc_list[i]);
        if(UCDataset::object_info.count(uc) > 0)
        {
            new_ucd->object_info[uc] = UCDataset::object_info[uc];
        }

        if(UCDataset::size_info.count(uc) > 0)
        {        
            new_ucd->size_info[uc] = UCDataset::size_info[uc];
        }
    }
    new_ucd->save_to_ucd(save_path);
}

void UCDataset::split(std::string ucd_part_a, std::string ucd_part_b, float ratio)
{
    
    if(ratio <= 0 || ratio >= 1)
    {
        std::cout << "ratio should in (0, 1)" << std::endl;
        throw "ratio should in (0, 1)";
    }
    
    std::vector<std::string> uc_list = UCDataset::uc_list;
    std::random_shuffle(uc_list.begin(), uc_list.end());
    int count_for_a = ratio * uc_list.size();
    
    // ucd_a
    UCDataset* ucd_a = new UCDataset(ucd_part_a);
    ucd_a->dataset_name = UCDataset::dataset_name;
    ucd_a->model_name = UCDataset::model_name;
    ucd_a->model_version = UCDataset::model_version;
    ucd_a->describe = UCDataset::describe;
    ucd_a->add_time = -1;
    ucd_a->update_time = -1;
    ucd_a->label_used = UCDataset::label_used;
    for(int i=0; i<count_for_a; i++)
    {
        std::string uc = uc_list[i];
        ucd_a->uc_list.push_back(uc);
        if(UCDataset::object_info.count(uc) > 0)
        {
            ucd_a->object_info[uc] = UCDataset::object_info[uc];
        }
        if(UCDataset::size_info.count(uc) > 0)
        {
            ucd_a->size_info[uc] = UCDataset::size_info[uc];
        }
    }
    ucd_a->save_to_ucd(ucd_part_a);
    delete ucd_a;

    // ucd_b
    UCDataset* ucd_b = new UCDataset(ucd_part_b);
    ucd_b->dataset_name = UCDataset::dataset_name;
    ucd_b->model_name = UCDataset::model_name;
    ucd_b->model_version = UCDataset::model_version;
    ucd_b->describe = UCDataset::describe;
    ucd_b->add_time = -1;
    ucd_b->update_time = -1;
    ucd_b->label_used = UCDataset::label_used;
    for(int i=count_for_a; i<uc_list.size(); i++)
    {
        std::string uc = uc_list[i];
        ucd_b->uc_list.push_back(uc);
        if(UCDataset::object_info.count(uc) > 0)
        {
            ucd_b->object_info[uc] = UCDataset::object_info[uc];
        }
        if(UCDataset::size_info.count(uc) > 0)
        {
            ucd_b->size_info[uc] = UCDataset::size_info[uc];
        }
    }
    ucd_b->save_to_ucd(ucd_part_b);
    delete ucd_b;
}

void UCDataset::absorb(std::string meat_ucd, std::string save_path, std::string need_attr)
{
    UCDataset* ucd = new UCDataset(meat_ucd);
    ucd->parse_ucd(true);

    tqdm bar;
    int N = UCDataset::uc_list.size();
    for(int i=0; i<UCDataset::uc_list.size(); i++)
    {
        std::string uc = UCDataset::uc_list[i];
        if((need_attr == "size_info") || (need_attr == "all"))
        {
            if((UCDataset::size_info.count(uc) == 0) && (ucd->size_info.count(uc) > 0))
            {
                UCDataset::size_info[uc] = ucd->size_info[uc];
            }
        }
        else if((need_attr == "object_info") || (need_attr == "all"))
        {
            if((UCDataset::object_info.count(uc) == 0) && (ucd->object_info.count(uc) > 0))
            {
                UCDataset::object_info[uc] = ucd->object_info[uc];
            }
        }
        else
        {
            std::cout << "attr_name not support, [size_info, object_info]";
            throw "attr_name not support, [size_info, object_info]";
        }
        bar.progress(i, N);
    }
    bar.finish();
    UCDataset::save_to_ucd(save_path);
    delete ucd;
}

void UCDataset::devide(std::string save_path, int devide_count)
{
    if(! (save_path.substr(save_path.size()-5) == ".json"))
    {
        std::cout << ERROR_COLOR << "save_path error, need save_path like save_dir/save_name + .json" << STOP_COLOR << std::endl;
        // throw "save_path error, need like save_dir/save_name + .json";
        return;
    }

    if(devide_count > UCDataset::uc_list.size())
    {
        std::cout << "devide_count >= uc_list.size !" << std::endl;
        // throw "devide_count >= uc_list.size !";
        return;
    }

    std::string save_dir = get_file_folder(save_path);
    std::string save_name = get_file_name(save_path);
    if(! is_dir(save_dir))
    {
        std::cout << "save_dir not exists, " << save_dir << std::endl;
        // throw "save_dir not exists";    
        return;   
    }

    std::vector<std::string> uc_list = UCDataset::uc_list;
    std::random_shuffle(uc_list.begin(), uc_list.end());

    // devide uc_list
    std::map< int, std::vector<std::string> > uc_list_map;
    int uc_size = uc_list.size();
    for(int i=0; i<uc_list.size(); i++)
    {
        int index = i % devide_count;
        uc_list_map[index].push_back(uc_list[i]);
    }

    tqdm bar;
    int N = uc_list.size();

    // devide other info 
    int index = 0;
    for(int i=0; i<devide_count; i++)
    {
        std::string save_path = save_dir + "/" + save_name + "_" + std::to_string(i) + ".json";
        UCDataset* new_ucd      = new UCDataset(save_path);
        new_ucd->dataset_name   = UCDataset::dataset_name;
        new_ucd->model_version  = UCDataset::model_version;
        new_ucd->model_name     = UCDataset::model_name;
        new_ucd->describe       = UCDataset::describe;
        new_ucd->add_time       = UCDataset::add_time;
        new_ucd->update_time    = -1;
        new_ucd->label_used     = UCDataset::label_used;

        for(int j=0; j<uc_list_map[i].size(); j++)
        {
            std::string uc = uc_list_map[i][j];
            new_ucd->uc_list.push_back(uc);
            
            if(UCDataset::size_info.count(uc) > 0)
            {
                new_ucd->size_info[uc] = UCDataset::size_info[uc];
            }

            if(UCDataset::object_info.count(uc) > 0)
            {
                new_ucd->object_info[uc] = UCDataset::object_info[uc];
            }
            bar.progress(index, N);
            index += 1;
        }
        new_ucd->save_to_ucd(save_path);
        delete new_ucd;
    }
    bar.finish();
}

static std::vector<std::string> command_token(std::string line)
{

    // todo clean command first 
    // format command second  

    std::vector<std::string> res;

    std::string w = "";
    for(int i=0; i<line.size(); i++)
    {
        if(line[i] == ' ')
        {
            if(w == "")
            {
                continue;
            }
            else
            {
                res.push_back(w);
                w = "";
            }
        }
        else if(i == line.size()-1)
        {
            if(line[i] != ' ')
            {
                w += line[i];
                res.push_back(w);
            }
        }
        else
        {
            w += line[i];
        }
    }

    for(int i=0; i<res.size(); i++)
    {
        std::cout << res[i];
        std::cout << " - ";
    }
    std::cout << std::endl;

    return res;
}

static std::vector< std::vector< double > > get_points_from_str(std::string point_str)
{
    std::vector< std::vector< double > > points;
    bool is_end = false;
    bool is_start = true;
    std::string one_point_str = "";
    for(int i=1; i<point_str.size()-1; i++)
    {

        if(point_str[i] == '(')
        {
            is_start = true;
            continue;
        }
        else if(point_str[i] == ')')
        {
            is_end = true;
            is_start = false;
        }
        else if(point_str[i] == ',')
        {
            if(is_start)
            {
                one_point_str += point_str[i];
                continue;
            }
        }
        else
        {
            if(is_start)
            {
                one_point_str += point_str[i];
            }
            continue;
        }

        if(is_end)
        {
            std::vector< std::string > one_point = pystring::split(one_point_str, ",");
            double x = std::stof(one_point[0]);
            double y = std::stof(one_point[1]);
            points.push_back({x, y});
            one_point_str = "";
            is_end = false;
        }
    }
    return points;
}

static bool command_check(std::string command_path)
{
    // exec 
    std::ifstream infile; 
    infile.open(command_path);   
    assert(infile.is_open());

    std::string line;
    int index = 0;
    while(getline(infile, line))
    {
        index += 1;
        std::vector<std::string> tokens = command_token(line);

        if(tokens.size() == 0)
        {
            continue;
        }
        else if(pystring::startswith(tokens[0], "//"))
        {
            // 识别为注释
            continue;
        }
        else if(tokens.size() < 3)
        {
            std::cout << ERROR_COLOR << "line : " << index << ", format error : " << line << STOP_COLOR << std::endl;
            return false;
        }
        else if((tokens[0] == "ADD")    &&  (tokens[1] == "UC"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index <<  ", format error, ADD UC test_uc : " << line << STOP_COLOR << std::endl;
                return false;
            }
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;
            }
        }
        else if((tokens[0] == "ADD")    &&  (tokens[1] == "SIZE_INFO"))
        {
            if(tokens.size() != 5)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, ADD SIZE_INFO test_uc width height: " << line << STOP_COLOR << std::endl;
                return false;                
            }
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;                
            }

            // width, height
            try
            {
                int width = std::stoi(tokens[3]);
                int height = std::stoi(tokens[4]);

                if((width < 2) || (height < 2))
                {
                    std::cout << "line : " << index << "width or height too small : " << width << ", " << height << std::endl;
                    return false;
                }
            }
            catch(...)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, parse width,height error : " << tokens[3] << ", " << tokens[4] << STOP_COLOR << std::endl;
                return false;
            }

        }
        else if((tokens[0] == "ADD")    &&  (tokens[1] == "OBJECT_INFO"))
        {
            if(tokens.size() != 7)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, ADD OBJECT_INFO test_uc shape_type tag conf points : " << line << STOP_COLOR << std::endl;
                return false;              
            }
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;                     
            }
            else if((tokens[3] != "rectangle") && (tokens[3] != "line") && (tokens[3] != "linestrip") && (tokens[3] != "circle") && (tokens[3] != "point") && (tokens[3] != "polygon"))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal shape_type(point, line, linestrip, circle, rectangle, polygon) : " << tokens[2] << STOP_COLOR << std::endl;
                return false;       
            }

            // get conf
            try
            {
                float conf = std::stof(tokens[5]);
                if((conf < 0) || (conf > 1))
                {
                    std::cout << ERROR_COLOR << "line : " << index << ", format error, 0 < conf < 1 : " << tokens[5] << STOP_COLOR << std::endl;
                    return false;
                }
            }
            catch(...)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, parse conf error : " << tokens[5] << STOP_COLOR << std::endl;
                return false;
            }
            
            // get points 
            try
            {
                std::vector< std::vector< double > > points = get_points_from_str(tokens[6]);
            }
            catch(...)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, parse points error : " << tokens[6] << STOP_COLOR << std::endl;
                return false; 
            }
        }
        else if((tokens[0] == "SET")    &&  (tokens[1] == "DATASET_NAME"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, SET DATASET_NAME dataset_name" << STOP_COLOR << std::endl;
                return false;
            }
        }
        else if((tokens[0] == "SET")    &&  (tokens[1] == "MODEL_NAME"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, SET MODEL_NAME model_name" << STOP_COLOR << std::endl;
                return false;
            }  
        }
        else if((tokens[0] == "SET")    &&  (tokens[1] == "MODEL_VERSION"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, SET MODERL_VERSION model_version" << STOP_COLOR << std::endl;
                return false;
            } 
        }
        else if((tokens[0] == "SET")    &&  (tokens[1] == "DESCRIBE"))
        {
            if(tokens.size() < 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, SET DESCRIBE describ" << STOP_COLOR << std::endl;
                return false;
            }  
        }
        else if((tokens[0] == "SET")    &&  (tokens[1] == "LABEL_USED"))
        {
            if(tokens.size() < 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, SET LABEL_USED label_used" << STOP_COLOR << std::endl;
                return false;
            } 
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "UC"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, DROP UC uc" << STOP_COLOR << std::endl;
                return false;
            }
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;   
            }
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "SIZE_INFO"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, DROP SIZE_INFO uc" << STOP_COLOR << std::endl;
                return false;
            }   
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;   
            }
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "OBJECT_INFO"))
        {
            // 删除一个元素
            if(tokens.size() != 7)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, DROP OBJECT_INFO test_uc shape_type tag conf points" << STOP_COLOR << std::endl;
                return false;
            }   
            else if(! is_uc(tokens[2]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[2] << STOP_COLOR << std::endl;
                return false;   
            }
            else if((tokens[3] != "rectangle") && (tokens[3] != "line") && (tokens[3] != "linestrip") && (tokens[3] != "circle") && (tokens[3] != "point") && (tokens[3] != "polygon"))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal shape_type(point, line, linestrip, circle, rectangle, polygon) : " << tokens[2] << STOP_COLOR << std::endl;
                return false;       
            }

            // get conf
            try
            {
                float conf = std::stof(tokens[5]);
                if((conf < 0) || (conf > 1))
                {
                    std::cout << ERROR_COLOR << "line : " << index << ", format error, 0 < conf < 1 : " << tokens[5] << STOP_COLOR << std::endl;
                    return false;
                }
            }
            catch(...)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, parse conf error : " << tokens[5] << STOP_COLOR << std::endl;
                return false;
            }
            
            // get points 
            try
            {
                std::vector< std::vector< double > > points = get_points_from_str(tokens[6]);
            }
            catch(...)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, parse points error : " << tokens[6] << STOP_COLOR << std::endl;
                return false; 
            }
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "ALL") && (tokens[2] == "UC"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, DROP ALL UC" << STOP_COLOR << std::endl;
                return false;
            }
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "ALL") && (tokens[2] == "SIZE_INFO"))
        {
            if(tokens.size() != 3)
            {
                std::cout << ERROR_COLOR << "line : " << index << ", format error, DROP ALL SIZE_INFO" << STOP_COLOR << std::endl;
                return false;
            }        
        }
        else if((tokens[0] == "DROP")   &&  (tokens[1] == "ALL") && (tokens[2] == "OBJECT_INFO"))
        {
            // 删除 uc 对应的所有的 obj
            if(tokens.size() != 4)
            {
                std::cout << ERROR_COLOR << "line : " << index <<  ", format error, DROP ALL OBJECT_INFO uc : " << line << STOP_COLOR << std::endl;
                return false;   
            }
            else if(! is_uc(tokens[3]))
            {
                std::cout << ERROR_COLOR << "line : "<< index << ", format error, illeagal uc : " << tokens[3] << STOP_COLOR << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << ERROR_COLOR << "line : " << index << ", gramma not support : " << line << STOP_COLOR << std::endl;
            return false;
        }
    }
    infile.close();
    return true;
}

void UCDataset::command_ADD(std::vector<std::string> tokens)
{
    std::string command = tokens[1];
    if(command == "UC")
    {
        // needn-t format check, do it before 
        std::string uc = tokens[2];
        UCDataset::uc_list.push_back(uc);
    }
    else if(command == "SIZE_INFO")
    {
        std::string uc  = tokens[2];
        int width       = std::stoi(tokens[3]);
        int height      = std::stoi(tokens[4]);
        UCDataset::size_info[uc] = {width, height};
    }
    else if(command == "OBJECT_INFO")
    {
        std::string uc          = tokens[2];
        std::string shape_type  = tokens[3];
        std::string label       = tokens[4];
        float       conf        = std::stof(tokens[5]);
        std::vector< std::vector< double > > points = get_points_from_str(tokens[6]);
        //  
        LabelmeObjFactory label_factory;
        LabelmeObj* obj = label_factory.CreateObj(shape_type);
        obj->conf   = conf;
        obj->label  = label;
        obj->points = points;
        UCDataset::add_obj(uc, obj);
    }
    else
    {
        std::cout << ERROR_COLOR << command << " not support" << STOP_COLOR << std::endl;
        throw command + " not support";
    }
}

void UCDataset::command_DROP(std::vector<std::string> tokens)
{
    std::string command = tokens[1];
    if(command == "ALL"      && tokens[2] == "UC")
    {
        std::string uc = tokens[2];
        UCDataset::uc_list.clear();
        UCDataset::size_info.clear();
        UCDataset::object_info.clear();
    }
    else if(command == "ALL" && tokens[2] == "SIZE_INFO")
    {
        std::string uc = tokens[2];
        UCDataset::size_info.clear();
    }
    else if(command == "ALL" && tokens[2] == "OBJECT_INFO")
    {
        std::string uc = tokens[2];
        UCDataset::object_info.clear();
    }
    else if(command == "UC")
    {
        std::string uc = tokens[2];
        // uc_list
        for(int i=0; i<UCDataset::uc_list.size(); i++)
        {
            if(UCDataset::uc_list[i] == uc)
            {
                UCDataset::uc_list.erase(UCDataset::uc_list.begin() + i);
                break;
            }
        }
        UCDataset::size_info.erase(uc);
        UCDataset::object_info.erase(uc);
    }
    else if(command == "SIZE_INFO")
    {
        std::string uc = tokens[2];
        UCDataset::size_info.erase(uc);
    }
    else if(command == "OBJECT_INFO")
    {
        std::string uc          = tokens[2];
        std::string shape_type  = tokens[3];
        std::string label       = tokens[4];
        float       conf        = std::stof(tokens[5]);
        std::vector< std::vector< double > > points = get_points_from_str(tokens[6]);
        

        LabelmeObjFactory label_factory;
        LabelmeObj* obj = label_factory.CreateObj(shape_type);
        obj->conf = conf;
        obj->label = label;
        obj->points = points;
        UCDataset::delete_obj(uc, obj);
    }
}

void UCDataset::command_SET(std::vector<std::string> tokens)
{
    std::string command = tokens[1];
    
    if(command == "DATASET_NAME")
    {
        UCDataset::dataset_name = tokens[2];
    }
    else if(command == "MODEL_NAME")
    {
        UCDataset::model_name = tokens[2];
    }
    else if(command == "MODEL_VERSION")
    {
        UCDataset::model_version = tokens[2];
    }
    else if(command == "DESCRIBE")
    {
        UCDataset::describe.clear();
        for(int i=2; i<tokens.size(); i++)
        {
            UCDataset::describe += " " + tokens[i];
        }
    }
    else if(command == "LABEL_USED")
    {
        UCDataset::label_used.clear();
        for(int i=2; i<tokens.size(); i++)
        {
            UCDataset::label_used.push_back(tokens[i]);
        }
    }
}

void UCDataset::exec(std::string command_path)
{
    if(! is_file(command_path))
    {
        std::cout << "command_path not exists" << std::endl;
        return;
    }

    bool check_res = command_check(command_path);
    
    if(check_res == false)
    {
        std::cout << ERROR_COLOR << "command format error " << STOP_COLOR << std::endl;
        return;
    }

    // exec 
    std::ifstream infile; 
    infile.open(command_path);   
    assert(infile.is_open());

    std::string line;
    while(getline(infile, line))
    {
        std::cout << line << std::endl;
        std::vector<std::string> tokens = command_token(line);

        if(tokens.size() == 0)
        {
            // 跳过空行
            continue;
        }
        else if(tokens[0] == "ADD")
        {
            UCDataset::command_ADD(tokens);
        }
        else if(tokens[0] == "DROP")
        {
            UCDataset::command_DROP(tokens);
        }
        else if(tokens[0] == "DROP_ALL")
        {
            // UCDataset::command_DROP_ALL(tokens);
        }
        else if(tokens[0] == "SET")
        {
            UCDataset::command_SET(tokens);
        }
        else if(pystring::startswith(tokens[0], "//"))
        {
            // 注释
            continue;
        }
        else
        {
            std::cout << ERROR_COLOR << "不存在的方法 : " << tokens[0] << STOP_COLOR << std::endl;
        }
    }
    infile.close();
}

void UCDataset::drop_tags(std::set<std::string> tags)
{
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::vector<LabelmeObj*> objs;
        for(int i=0; i<iter->second.size(); i++)
        {
            LabelmeObj* obj = iter->second[i];
            if(tags.count(obj->label) == 0)
            {
                objs.push_back(obj);
                // delete obj;
            }
        }
        iter->second = objs;
        if(objs.size() == 0)
        {
            UCDataset::size_info.erase(iter->first);
        }
        iter++;
    }
}

void UCDataset::update_tags(std::map< std::string, std::string > tag_map)
{
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            LabelmeObj* obj = iter->second[i];
            if(tag_map.count(obj->label) > 0)
            {
                obj->label = tag_map[obj->label];
            }
        }
        iter++;
    }
}

void UCDataset::drop_empty_uc()
{
    std::vector< std::string > uc_list;
    for(int i=0; i<UCDataset::uc_list.size(); i++)
    {
        std::string uc = UCDataset::uc_list[i];
        if(UCDataset::object_info[uc].size() > 0)
        {
            uc_list.push_back(uc);
        }
    }
    UCDataset::uc_list = uc_list;
}

std::set<std::string> UCDataset::get_tags()
{
    auto iter = UCDataset::object_info.begin();
    std::set<std::string> tags;
    while(iter != UCDataset::object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            LabelmeObj* obj = iter->second[i]; 
            tags.insert(obj->label);
        }
        iter++;
    }
    return tags;
}

void UCDataset::save_to_huge_ucd(std::string save_dir, std::string save_name, int volume_index)
{
    //  uci 和 obi 两个文件进行保存

    nlohmann::json uci_info = {
        {"dataset_name",    UCDataset::dataset_name},
        {"model_name",      UCDataset::model_name},
        {"model_version",   UCDataset::model_version},
        {"add_time",        UCDataset::add_time},
        {"update_time",     UCDataset::update_time},
        {"describe",        UCDataset::describe},
        {"label_used",      UCDataset::label_used},
        {"uc_list",         UCDataset::uc_list},
        {"volume_size",     UCDataset::volume_size},
    };

    std::map<std::string, std::vector<nlohmann::json> > shapes_info;
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        std::vector<LabelmeObj*> obj_vector = iter->second; 
        for(int i=0; i<iter->second.size(); i++)
        {
            nlohmann::json each_obj;
            each_obj["shape_type"]  = obj_vector[i]->shape_type;
            each_obj["label"]       = obj_vector[i]->label;
            each_obj["points"]      = obj_vector[i]->points;
            each_obj["conf"]        = obj_vector[i]->conf;
            shapes_info[iter->first].push_back(each_obj);

        }
        iter++;
    }  

    std::string uci_path, obi_path, szi_path;
    if(volume_index == 0)
    {
        uci_path = save_dir + "/" + save_name + ".uci";
        obi_path = save_dir + "/" + save_name + ".obi";
        szi_path = save_dir + "/" + save_name + ".szi";
    }
    else
    {
        uci_path = save_dir + "/" + save_name + ".u" + std::to_string(volume_index);
        obi_path = save_dir + "/" + save_name + ".o" + std::to_string(volume_index);
        szi_path = save_dir + "/" + save_name + ".s" + std::to_string(volume_index);
    }
    
    // uci_info
    std::ofstream o1(uci_path);
    o1 << std::setw(4) << uci_info << std::endl;
    
    // obi_info
    nlohmann::json obi_info = {};
    obi_info["shapes"] = shapes_info;
    std::ofstream o2(obi_path);
    o2 << std::setw(4) << obi_info << std::endl;

    // szi_info
    nlohmann::json size_info = {};
    size_info["size_info"] = UCDataset::size_info;
    std::ofstream o3(szi_path);
    o3 << std::setw(4) << size_info << std::endl;
}

void UCDataset::clear_obj_info()
{
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            delete iter->second[i];
        }
        iter++;
    }
    UCDataset::object_info.clear();
}

void UCDataset::load_uci(std::string uci_path)
{
    if(! is_file(uci_path))
    {
        std::cout << ERROR_COLOR << "uci path not exists : " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    UCDataset::volumn_dir = get_file_folder(uci_path);
    UCDataset::volume_name = get_file_name(uci_path);
    UCDataset::volume_count = 1;

    int index = 0;
    while(true)
    {
        index += 1;
        std::string volume_uci_path = get_uci_path(index);
        if(is_file(volume_uci_path))
        {
            UCDataset::volume_count += 1;
        }
        else
        {
            break;
        }
    }
}

void UCDataset::parse_volume(int volumn_index, bool parse_szi, bool parse_obi, bool clear_obj)
{
    // 数据清空
    UCDataset::uc_list.clear();
    UCDataset::size_info.clear();
    if(clear_obj)
    {
        UCDataset::clear_obj_info();
    }
    else
    {
        UCDataset::object_info.clear();
    }

    std::string uci_path;
    if(volumn_index == 0)
    {
        uci_path = UCDataset::get_uci_path(0);
        if(! is_file(uci_path))
        {
            std::cout << ERROR_COLOR<< "uci path not exists : " << uci_path << STOP_COLOR << std::endl;
            return;
        }

        std::ifstream jsfile(uci_path);
        json data = json::parse(jsfile); 

        auto dataset_name   = data["dataset_name"];
        auto model_name     = data["model_name"];
        auto model_version  = data["model_version"];
        auto add_time       = data["add_time"];
        auto update_time    = data["update_time"];
        auto describe       = data["describe"];
        auto label_used     = data["label_used"];
        auto uc_list        = data["uc_list"];
        auto volume_size    = data["volume_size"];

        if(dataset_name     != nullptr) { UCDataset::dataset_name = dataset_name; }
        if(model_name       != nullptr) { UCDataset::model_name = model_name; }
        if(model_version    != nullptr) { UCDataset::model_version = model_version; }
        if(add_time         != nullptr) { UCDataset::add_time = add_time; }
        if(update_time      != nullptr) { UCDataset::update_time = update_time; }
        if(describe         != nullptr) { UCDataset::describe = describe; }
        if(label_used       != nullptr) { UCDataset::label_used = label_used; }
        if(uc_list          != nullptr) { UCDataset::uc_list = uc_list; }
        if(volume_size      != nullptr) { UCDataset::volume_size = volume_size; }
        UCDataset::unique();
    }
    else
    {
        uci_path = get_uci_path(volumn_index);
        std::ifstream jsfile(uci_path);
        json data = json::parse(jsfile); 
        
        auto uc_list = data["uc_list"];
        
        if(uc_list          != nullptr) { UCDataset::uc_list = uc_list; }
        UCDataset::unique();
    }

    if(parse_szi)
    {
        std::string szi_path;
        szi_path = get_szi_path(volumn_index);

        std::ifstream jsfile(szi_path);
        json data = json::parse(jsfile); 

        auto size_info      = data["size_info"];
        if(size_info        != nullptr) { UCDataset::size_info = size_info; }
    }

    if(parse_obi)
    {
        std::string obi_path;
        obi_path = get_obi_path(volumn_index);

        if(! is_file(obi_path))
        {
            std::cout << "obi path not exists : " << obi_path << std::endl;
            return;
        }

        std::ifstream jsfile(obi_path);
        json data = json::parse(jsfile); 

        auto shapes_info = data["shapes"];
        LabelmeObjFactory obj_factory;

        if(shapes_info != nullptr)
        {
            int N = shapes_info.size();
            int index = 0;
            auto iter = shapes_info.begin();
            while(iter != shapes_info.end())
            {
                index += 1;
                std::string uc = iter.key();
                for(int i=0; i<iter.value().size(); i++)
                {
                    std::string shape_type = iter.value()[i]["shape_type"];
                    std::string label = iter.value()[i]["label"];
                    std::vector< std::vector<double> > points = iter.value()[i]["points"];
                    auto conf = iter.value()[i]["conf"];
                    LabelmeObj* obj = obj_factory.CreateObj(shape_type);
                    obj->label = label;
                    obj->points = points;

                    if(conf == nullptr)
                    {
                        obj->conf = -1;                      
                    }
                    else
                    {
                        obj->conf = conf;
                    }

                    UCDataset::object_info[uc].push_back(obj);
                }
                iter++;
            }
        }
    }
}

std::string UCDataset::get_uci_path(int index)
{
    if(UCDataset::volume_name == "" | UCDataset::volumn_dir == "" | UCDataset::volume_count == 0)
    {
        std::cout << ERROR_COLOR << "volume_name volume_dir is empty" << STOP_COLOR << std::endl;
    }

    std::string uci_path;
    if(index == 0)
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".uci";
    }
    else
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".u" + std::to_string(index);
    }
    return uci_path;
}

std::string UCDataset::get_obi_path(int index)
{
    if(UCDataset::volume_name == "" | UCDataset::volumn_dir == "" | UCDataset::volume_count == 0)
    {
        std::cout << ERROR_COLOR << "volume_name volume_dir is empty" << STOP_COLOR << std::endl;
    }

    std::string uci_path;
    if(index == 0)
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".obi";
    }
    else
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".o" + std::to_string(index);
    }
    return uci_path;
}

std::string UCDataset::get_szi_path(int index)
{
    if(UCDataset::volume_name == "" | UCDataset::volumn_dir == "" | UCDataset::volume_count == 0)
    {
        std::cout << ERROR_COLOR << "volume_name volume_dir is empty" << STOP_COLOR << std::endl;
    }

    std::string uci_path;
    if(index == 0)
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".szi";
    }
    else
    {
        uci_path = UCDataset::volumn_dir + "/" + UCDataset::volume_name + ".s" + std::to_string(index);
    }
    return uci_path;
}

void UCDataset::to_uci(std::string uci_path, int volume_size)
{
    std::string save_dir    = get_file_folder(uci_path);
    std::string save_name   = get_file_name(uci_path);
    int volume_count        = 0;
    int info_count          = 0;
    UCDataset* each_ucd     = new UCDataset();
    each_ucd->volume_size   = volume_size;

    for(int i=0; i<UCDataset::uc_list.size(); i++)
    {
        std::string uc = UCDataset::uc_list[i];
        // uc
        info_count += 1;
        each_ucd->uc_list.push_back(uc);
        // size info
        if(UCDataset::size_info.count(uc) > 0)
        {
            info_count += 2;
            each_ucd->size_info[uc] = UCDataset::size_info[uc];
        }
        // obj_info
        if(UCDataset::object_info.count(uc) > 0)
        {
            info_count += UCDataset::object_info[uc].size() * 4;
            each_ucd->object_info[uc] = UCDataset::object_info[uc];
        }
        // save uci
        if(info_count > (volume_size * 1000 * 100))
        {
            each_ucd->save_to_huge_ucd(save_dir, save_name, volume_count);
            std::cout << "save volume : " << std::to_string(volume_count) << std::endl;
            volume_count   += 1;
            info_count      = 0;
            // clear each_ucd 
            each_ucd->clear_obj_info();
            each_ucd->object_info.clear();
            each_ucd->uc_list.clear();
            each_ucd->size_info.clear();
        }
    }
    // the rest
    if(info_count > 0)
    {
        each_ucd->save_to_huge_ucd(save_dir, save_name, volume_count);
        std::cout << "save volume : " << std::to_string(volume_count) << std::endl;
    }
    delete each_ucd;
}

int UCDataset::get_info_count()
{
    int info_count = 0;
    info_count += UCDataset::uc_list.size();
    info_count += UCDataset::size_info.size() * 2;
    
    auto iter = UCDataset::object_info.begin();
    while(iter != UCDataset::object_info.end())
    {
        info_count += iter->second.size() * 4;
        iter++;
    }
    return info_count;
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
        UCDatasetUtil::cache_img_dir    = UCDatasetUtil::cache_dir + "/" + "img_cache";
        UCDatasetUtil::cache_xml_dir    = UCDatasetUtil::cache_dir + "/" + "xml_cache";
        UCDatasetUtil::cache_crop_dir   = UCDatasetUtil::cache_dir + "/" + "crop_cache";
        UCDatasetUtil::color_file       = UCDatasetUtil::cache_dir + "/" + "color.txt";

        if(! is_dir(UCDatasetUtil::cache_img_dir))
        {
            create_folder(UCDatasetUtil::cache_img_dir);
        }

        if(! is_dir(UCDatasetUtil::cache_xml_dir))
        {
            create_folder(UCDatasetUtil::cache_xml_dir);
        }

        if(! is_dir(UCDatasetUtil::cache_crop_dir))
        {
            create_folder(UCDatasetUtil::cache_crop_dir);
        }

        if(! is_file(UCDatasetUtil::color_file))
        {
            // 创建这个文件，里面加上一个颜色 demo
            std::ofstream OutFile(UCDatasetUtil::color_file); 
            OutFile << "# 默认颜色, test_color, (R,G,B)" << std::endl;
            OutFile << "default,0,255,0" << std::endl;
            OutFile.close();
        }
    }
}

void UCDatasetUtil::save_img_xml_json(std::string save_dir, bool need_img, bool need_xml, int need_count)
{
    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir not exists : " << save_dir << STOP_COLOR << std::endl;
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

    if(! is_file(save_path))
    {
        httplib::Client cli(UCDatasetUtil::root_url);
        auto res = cli.Get(url);

        if(res == nullptr)
        {
            std::cout << ERROR_COLOR << "connect error : " << url << STOP_COLOR << std::endl;
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
            std::cout << ERROR_COLOR << "load error : " << url << STOP_COLOR << std::endl;
            std::cout << ERROR_COLOR << "error info : " << res->body << STOP_COLOR << std::endl;
        }
    }
}

void UCDatasetUtil::load_img(std::string save_dir, std::vector<std::string> uc_list)
{
    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save dir not exists";
    }

    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};

    tqdm bar;
    int N = uc_list.size();
    for(int i=0; i<uc_list.size(); i++)
    {
        std::string img_url = "/file/" + uc_list[i] + ".jpg";
        std::string save_img_path = save_dir + "/" + uc_list[i] + ".jpg";  
                
        if(! is_file(save_img_path))
        {
            std::string img_cache_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc_list[i], suffix);
            if(is_file(img_cache_path))
            {
                if(is_read_file(img_cache_path))
                {
                    copy_file(img_cache_path, save_img_path);
                }
                else
                {
                    std::cout << WARNNING_COLOR << "img path dont have read access : " << img_cache_path << STOP_COLOR << std::endl;
                }
            }
            else
            {
                UCDatasetUtil::load_file(img_url, save_img_path, i); 
            }
        }
        bar.progress(i, N);
    }
    bar.finish();
}

void UCDatasetUtil::load_img_with_assign_uc(std::string save_dir, std::string uc)
{

    // todo 这个函数有问题，好像下载下来的数据都是空的
    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save dir not exists";
    }

    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::string img_cache_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, suffix);
    std::string save_img_path = save_dir + "/" + uc + ".jpg";  
    std::string img_url = "/file/" + uc + ".jpg";


    // std::cout << "-------------" << std::endl;
    // std::cout << "save_dir : " << save_dir << std::endl;
    // std::cout << "uc : " << uc << std::endl;
    // std::cout << "img_url : " << img_url << std::endl;
    // std::cout << "save_img_path : " << save_img_path << std::endl;
    // std::cout << "img_cache_path : " << img_cache_path << std::endl;
    // std::cout << "-------------" << std::endl;

    if(! is_file(save_img_path))
    {
        if(is_file(img_cache_path))
        {
            copy_file(img_cache_path, save_img_path);
        }
        else
        {
            UCDatasetUtil::load_file(img_url, save_img_path); 
        }
    }
}

void UCDatasetUtil::load_xml(std::string save_dir, std::vector<std::string> uc_list)
{
    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save dir not exists";
    }

    tqdm bar;
    int N = uc_list.size();
    for(int i=0; i<uc_list.size(); i++)
    {
        std::string xml_url = "/file/" + uc_list[i] + ".xml";
        std::string save_xml_path = save_dir + "/" + uc_list[i] + ".xml";  
        if(! is_file(save_xml_path))
        {
            UCDatasetUtil::load_file(xml_url, save_xml_path, i); 
        }
        // else
        // {
        //     std::cout << i <<  ", file exists : " << save_xml_path << std::endl;
        // }
        bar.progress(i, N);
    }
    bar.finish();
}

void UCDatasetUtil::load_ucd(std::string ucd_name, std::string save_path)
{
    UCDatasetUtil::load_file("/ucd/" + ucd_name, save_path);
}

void UCDatasetUtil::load_ucd_app(std::string version, std::string save_path)
{
    UCDatasetUtil::load_file("/ucd_app/" + version, save_path);
}

void UCDatasetUtil::search_ucd()
{
    std::string check_url = "http://" + UCDatasetUtil::host + ":" + std::to_string(UCDatasetUtil::port);
    httplib::Client cli(check_url);
    auto res = cli.Get("/ucd/check");
    
    if(res != nullptr)
    {
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
    else
    {
        std::cout << ERROR_COLOR << "connect error : " << check_url << STOP_COLOR << std::endl;
    }
}

void UCDatasetUtil::delete_ucd(std::string std_name)
{
    httplib::Client cli(UCDatasetUtil::root_url);
    auto res = cli.Delete("/ucd/delete/" + std_name + ".json");
    if(res == nullptr)
    {
        std::cout << ERROR_COLOR << "delete failed !" << STOP_COLOR << std::endl;
        throw "delete failed !";
    }
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
        // 这边返回 status 不为 200 也报错？        
        if(resSendFiles == nullptr)
        {
            std::cout << ERROR_COLOR << "upload failed !" << STOP_COLOR << std::endl;
            throw "upload failed !";
        }
    }
    else
    {
        httplib::MultipartFormDataItems items = {{"json_file", buffer_lf_img.str(), ucd_name_suffix, "application/octet-stream"},{"ucd_name", ucd_name}};
        auto resSendFiles = cliSendFiles.Post("/ucd/upload", items);
        if(resSendFiles == nullptr)
        {
            std::cout << ERROR_COLOR << "upload failed !" << STOP_COLOR << std::endl;
            throw "upload failed !";
        }
    }
}

void UCDatasetUtil::get_ucd_from_img_dir(std::string img_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> img_path_vector = get_all_file_path_recursive(img_dir, suffix);
    UCDataset* ucd = new UCDataset(ucd_path);

    tqdm bar;
    int N = img_path_vector.size();

    std::string uc;
    for(int i=0; i<img_path_vector.size(); i++)
    {
        uc = get_file_name(img_path_vector[i]);

        if(is_uc(uc))
        {
            // get uc
            // std::cout << img_path_vector[i] << std::endl;
            ucd->uc_list.push_back(uc);
            // get img size
            FILE *file = fopen(img_path_vector[i].c_str(), "rb");
            auto imageInfo = getImageInfo<IIFileReader>(file);
            fclose(file);
            int height = imageInfo.getHeight();
            int width =  imageInfo.getWidth();
            ucd->size_info[uc] = {width, height};
        }
        bar.progress(i, N);
    }
    bar.finish();
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_xml_dir(std::string xml_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".xml"};
    std::vector<std::string> xml_path_vector = get_all_file_path_recursive(xml_dir, suffix);
    UCDataset* ucd = new UCDataset(ucd_path);
    std::string uc;
    int is_uc_count  = 0; 
    int not_uc_count = 0;
    tqdm bar;
    int N = xml_path_vector.size();
    for(int i=0; i<xml_path_vector.size(); i++)
    {
        uc = get_file_name(xml_path_vector[i]);
        if(is_uc(uc))
        {
            ucd->add_voc_xml_info(uc, xml_path_vector[i]);
            is_uc_count += 1;
        }
        else
        {
            not_uc_count += 1;
        }
        bar.progress(i, N);
    }
    bar.finish();
    ucd->save_to_ucd(ucd_path);

    // print
    std::cout << "is  uc count : " << is_uc_count << std::endl;
    std::cout << "not uc count : " << not_uc_count << std::endl;

    delete ucd;
}

void UCDatasetUtil::get_ucd_from_huge_xml_dir(std::string xml_dir, std::string save_path, int volume_size)
{
    std::string save_dir    = get_file_folder(save_path);
    std::string save_name   = get_file_name(save_path);
    // UCDataset::volume_size  = volume_size;

    if(! is_write_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir is not writeable : " << save_dir << STOP_COLOR << std::endl;
        return;
    }

    if(! pystring::endswith(save_path, ".uci"))
    {
        std::cout << ERROR_COLOR << "save_path not end with .uci : " << STOP_COLOR << save_path << std::endl;
        return;
    }

    std::set<std::string> suffix {".xml"};
    std::vector<std::string> xml_path_vector = get_all_file_path_recursive(xml_dir, suffix);
    UCDataset* ucd = new UCDataset();
    ucd->volume_size = volume_size;
    std::string uc;

    tqdm bar;
    int N = xml_path_vector.size();

    int volume_count    = 0;
    int info_count      = 0;
    int is_uc_count     = 0; 
    int not_uc_count    = 0;
    
    for(int i=0; i<xml_path_vector.size(); i++)
    {
        info_count += 1;
        uc = get_file_name(xml_path_vector[i]);
        if(is_uc(uc))
        {
            ucd->add_voc_xml_info(uc, xml_path_vector[i]);
            is_uc_count += 1;

            if(ucd->object_info.count(uc) > 0)
            {
                info_count += ucd->object_info[uc].size() * 4;
            }

            if(ucd->size_info.count(uc) > 0)
            {
                info_count += ucd->size_info[uc].size() * 2;
            }
        }
        else
        {
            not_uc_count += 1;
        }
        bar.progress(i, N);

        // 保存一个分卷的数据
        if(info_count > (volume_size * 1000 * 100))
        {
            ucd->save_to_huge_ucd(save_dir, save_name, volume_count);
            volume_count += 1;
            info_count = 0;

            // empty ucd
            ucd->clear_obj_info();
            ucd->object_info.clear();
            ucd->size_info.clear();
            ucd->uc_list.clear();
        }
    }

    // the rest
    if(info_count > 0)
    {
        ucd->save_to_huge_ucd(save_dir, save_name, volume_count);
    }
    bar.finish();
    
    // print 
    std::cout << "is  uc count : " << is_uc_count << std::endl;
    std::cout << "not uc count : " << not_uc_count << std::endl;
    
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_json_dir(std::string json_dir, std::string ucd_path)
{
    std::set<std::string> suffix {".json"};
    std::vector<std::string> json_path_vector = get_all_file_path_recursive(json_dir, suffix);

    UCDataset* ucd = new UCDataset(ucd_path);
    std::string uc, json_path;
    tqdm bar;
    int N = json_path_vector.size();
    for(int i=0; i<json_path_vector.size(); i++)
    {
        uc = get_file_name(json_path_vector[i]);
        if(is_uc(uc))
        {
            // std::cout << i << ", add labelme json : " << json_path_vector[i] << std::endl;
            ucd->add_labelme_json_info(uc, json_path_vector[i]);
        }
        bar.progress(i, N);
    }
    bar.finish();
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_file_dir(std::string file_dir, std::string ucd_path)
{
    std::vector<std::string> json_path_vector = get_all_file_path_recursive(file_dir);

    UCDataset* ucd = new UCDataset(ucd_path);
    std::string uc, json_path;

    tqdm bar;
    int N = json_path_vector.size();
    for(int i=0; i<json_path_vector.size(); i++)
    {
        uc = get_file_name(json_path_vector[i]);
        if(is_uc(uc))
        {
            // std::cout << i << ", add file : " << json_path_vector[i] << std::endl;
            ucd->uc_list.push_back(uc);
        }
        bar.progress(i, N);
    }
    bar.finish();
    ucd->save_to_ucd(ucd_path);
    delete ucd;
}

void UCDatasetUtil::get_ucd_from_dete_server(std::string  dete_server_dir, std::string ucd_path, std::string save_path)
{

    if(! is_file(ucd_path))
    {
        std::cout << ERROR_COLOR << "illegal ucd path : " << ucd_path << STOP_COLOR << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(false);
    UCDataset* save_ucd = new UCDataset(save_path);

    tqdm bar;
    int N = ucd->uc_list.size();
    for(int i=0; i<ucd->uc_list.size(); i++)
    {
        bar.progress(i, N);
        std::string uc = ucd->uc_list[i];
        std::string dete_xml_path = dete_server_dir + "/" + uc.substr(0, 3) + "/" + uc + ".xml";

        if(! is_read_file(dete_xml_path))
        {
            std::cout << WARNNING_COLOR << "can't read " << dete_xml_path << STOP_COLOR << std::endl;
            save_ucd->uc_list.push_back(uc);
        }
        else
        {
            save_ucd->add_voc_xml_info(uc, dete_xml_path);
        }
        bar.progress(i, N);
    }
    bar.finish();
    save_ucd->save_to_ucd(save_path);
    delete ucd;
    delete save_ucd;
}

void UCDatasetUtil::merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector)
{
    if(ucd_path_vector.size() < 2)
    {
        std::cout << ERROR_COLOR << "ucd path vector count < 2" << STOP_COLOR << std::endl;
        throw "ucd path vector count < 2";
    }

    UCDataset* ucd = new UCDataset(save_path);
    for(int i=0; i<ucd_path_vector.size(); i++)
    {
        std::cout << "  * merge -> " << ucd_path_vector[i] << std::endl;
        ucd->add_ucd_info(ucd_path_vector[i]);
    }
    ucd->save_to_ucd(save_path);
    delete ucd;
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
    ucd1->parse_ucd(true);
    UCDataset* ucd2 = new UCDataset(ucd_path_2);
    ucd2->parse_ucd(true);
    UCDataset* ucd_res = new UCDataset(save_path);
    std::vector<std::string> uc_difference;
    // set_intersection
    std::set<std::string> uc_set1(ucd1->uc_list.begin(), ucd1->uc_list.end());
    std::set<std::string> uc_set2(ucd2->uc_list.begin(), ucd2->uc_list.end());
    std::set_difference(uc_set1.begin(), uc_set1.end(), uc_set2.begin(), uc_set2.end(), std::inserter(uc_difference, uc_difference.begin()));
    // 
    ucd_res->uc_list = uc_difference;
    // 
    for(int i=0; i<uc_difference.size(); i++)
    {
        std::string uc = uc_difference[i];

        if(ucd1->object_info.count(uc) > 0)
        {
            ucd_res->object_info[uc] = ucd1->object_info[uc];
        }

        if(ucd1->size_info.count(uc) > 0)
        {
            ucd_res->size_info[uc] = ucd1->size_info[uc];
        }
    }
    //
    ucd_res->save_to_ucd(save_path);
    // delete
    delete ucd1;
    delete ucd2;
    delete ucd_res;
}

bool UCDatasetUtil::is_ucd_path(std::string ucd_path)
{
    if((! is_file(ucd_path)) || (ucd_path.substr(ucd_path.size()-5, 5) != ".json"))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool UCDatasetUtil::is_uci_path(std::string uci_path)
{
    if(((! is_file(uci_path))) || (uci_path.substr(uci_path.size()-4, 4) != ".uci"))
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
        std::cout << ERROR_COLOR << "ucd path not exists : " << ucd_path << STOP_COLOR << std::endl;
        return;
    }
    int tag_count = 0;
    int dete_obj_count=0; 
    UCDataset* ucd = new UCDataset(ucd_path);
    // count_tags 函数中会自动 解析一遍 json 
    ucd->parse_ucd(true);
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
            std::cout << std::setw(15) << std::left << "[" + iter_count->first + "]" << std::setw(20) << std::left << iter->first  << " : " << iter->second << std::endl;
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

void UCDatasetUtil::count_volume_tags(std::string uci_path)
{
    if(! UCDatasetUtil::is_uci_path(uci_path))
    {
        std::cout << ERROR_COLOR << "uci path not exists or ios not uci path: " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset();
    ucd->load_uci(uci_path);

    std::map<std::string, std::map<std::string, int> > count_map = ucd->count_volume_tags();
    int tag_count = 0;
    int dete_obj_count = 0; 
    // print statistics res
    std::cout << "--------------------------------------------------------------" << std::endl;
    std::cout << std::setw(15) << std::left << "shape" << std::setw(20) << std::left << "tag" << std::setw(15) << std::left << "count" << "percentage(%)" << std::endl;
    std::cout << "--------------------------------------------------------------" << std::endl;
    
    // 
    auto iter_a = count_map.begin();
    while(iter_a != count_map.end())
    {
        auto iter_b = iter_a->second.begin();
        while(iter_b != iter_a->second.end())
        {
            dete_obj_count += iter_b->second;
            iter_b++;
        }
        iter_a++;
    }
    
    auto iter_count = count_map.begin();
    while(iter_count != count_map.end())
    {
        auto iter = iter_count->second.begin();
        while(iter != iter_count->second.end())
        {
            std::cout << std::setw(15) << std::left << "[" + iter_count->first + "]" << std::setw(20) << std::left << iter->first << std::setw(15) << std::left << iter->second << 100 * iter->second / (dete_obj_count * 1.0) << std::endl;
            tag_count += 1;
            iter++;
        }
        iter_count ++;
    }
    std::cout << "--------------------------------------------------------------" << std::endl;
    std::cout << "number of tag : " << tag_count << std::endl;
    std::cout << "number of obj : " << dete_obj_count << std::endl;
    std::cout << "--------------------------------------------------------------" << std::endl;
    delete ucd;
}

void UCDatasetUtil::cache_clear()
{

    if(! is_dir(UCDatasetUtil::cache_img_dir))
    {
        std::cout << "cache_img_dir not exists" << std::endl;
    }

    // 清空全部缓存
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> all_img_path_vector = get_all_file_path(UCDatasetUtil::cache_img_dir, suffix);
    
    tqdm bar;
    int N = all_img_path_vector.size();

    for(int i=0; i<all_img_path_vector.size(); i++)
    {
        remove(all_img_path_vector[i].c_str());
        // std::cout << i << " , remove : " << all_img_path_vector[i] << std::endl;
        bar.progress(i, N);
    }
    bar.finish();
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

    tqdm bar;
    int N = ucd->uc_list.size();

    for(int i=0; i<ucd->uc_list.size(); i++)
    {
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, ucd->uc_list[i], suffix);

        if(is_file(img_path))
        {
            remove(img_path.c_str());
            // std::cout << i << " , remove : " << img_path << std::endl;
        }
        bar.progress(i, N);
    }
    bar.finish();
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
        std::cout << ERROR_COLOR << "word folder not exists : " << save_dir << STOP_COLOR << std::endl;
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
            std::cout << ERROR_COLOR << "* 未找到字符 : " << words[i] << "，目前仅支持中文"<< STOP_COLOR << std::endl;
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

void UCDatasetUtil::cut_small_img(std::string ucd_path, std::string save_dir, bool is_split)
{
    // 
    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save dir not exists";
        return;
    }

    if(! UCDatasetUtil::is_ucd_path(ucd_path))
    {
        std::cout << ERROR_COLOR << "ucd_path not exists : " << ucd_path << STOP_COLOR << std::endl;
        throw "ucd_path not exists";
        return;
    }

    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);

    tqdm bar;
    int N = ucd->object_info.size();
    int uc_index = 0;
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        UCDatasetUtil::load_img_with_assign_uc(UCDatasetUtil::cache_img_dir, uc);
        std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        // std::string img_path = get_file_by_suffix_set("/home/ldq/ucd_dir/img", uc, img_suffix);

        // std::cout << "img_path : " << img_path << std::endl; 

        if(img_path == "")
        {
            std::cout << ERROR_COLOR << "load uc img failed : " << uc << STOP_COLOR << std::endl;
        }
        else
        {
            ucd->crop_dete_res_with_assign_uc(uc, img_path,  save_dir);
        }
        uc_index += 1;
        bar.progress(uc_index, N);
        iter++;
    }
    bar.finish();
    delete ucd;
}

void UCDatasetUtil::parse_labelme_json(std::string img_dir, std::string save_dir, std::string ucd_path)
{
    if(! is_dir(img_dir))
    {
        std::cout << ERROR_COLOR << "img_dir not exists : " << img_dir << STOP_COLOR << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save_dir not exists";
    }

    if(! is_file(ucd_path))
    {
        std::cout << ERROR_COLOR << "ucd_path not exists : " << ucd_path << STOP_COLOR << std::endl;
        throw "ucd_path not exists";
    }

    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    tqdm bar;
    int N = ucd->object_info.size();
    int index = 0;
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        UCDatasetUtil::load_img_with_assign_uc(UCDatasetUtil::cache_img_dir, uc);
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        
        if(is_file(img_path))
        {
            std::string json_path = save_dir + "/" + uc + ".json";
            if(is_file(json_path))
            {
                std::cout << ERROR_COLOR << index << ", json exists, ignore : " << json_path << STOP_COLOR << std::endl;
            }
            else
            {
                ucd->save_to_labelme_json_with_assign_uc(json_path, img_path, uc);
                std::cout << index << ", parse json : " << uc << std::endl;
            }
        }
        else
        {
            std::cout << ERROR_COLOR << "load img failed : " << img_path << STOP_COLOR << std::endl;
        }
        bar.progress(index, N);
        index += 1;
        iter ++;
    }
    bar.finish();
    delete ucd;
}

void UCDatasetUtil::parse_voc_xml(std::string img_dir, std::string save_dir, std::string ucd_path)
{
    if(! is_dir(img_dir))
    {
        std::cout << ERROR_COLOR << "img_dir not exists : " << img_dir << STOP_COLOR << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save_dir not exists";
    }

    if(! is_file(ucd_path))
    {
        std::cout << ERROR_COLOR << "ucd_path not exists : " << ucd_path << STOP_COLOR << std::endl;
        throw "ucd_path not exists";
    }

    tqdm bar;
    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    int index = 0;
    int N = ucd->object_info.size();
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        // UCDatasetUtil::load_img(UCDatasetUtil::cache_img_dir, uc);
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        std::string xml_path = save_dir + "/" + uc + ".xml";
        ucd->save_to_voc_xml_with_assign_uc(xml_path, img_path, uc);
        // std::cout << index << ", parse xml : " << uc << std::endl;
        bar.progress(index, N);
        index += 1;
        iter ++;
    }
    bar.finish();
    delete ucd;
}

void UCDatasetUtil::parse_volume_voc_xml(std::string img_dir, std::string save_dir, std::string uci_path)
{
    if(! is_dir(img_dir))
    {
        std::cout << ERROR_COLOR << "img_dir not exists : " << img_dir << STOP_COLOR << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save_dir not exists";
    }

    if(! UCDatasetUtil::is_uci_path(uci_path))
    {
        std::cout << ERROR_COLOR << "illegal uci_path : " << uci_path << STOP_COLOR << std::endl;
        throw "ucd_path not exists";
    }

    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset();
    ucd->load_uci(uci_path);

    tqdm bar;
    int N = ucd->volume_count;

    for(int i=0; i<ucd->volume_count; i++)
    {
        bar.progress(i, N);
        ucd->parse_volume(i, true, true, true);
        auto iter = ucd->object_info.begin();
        while(iter != ucd->object_info.end())
        {
            std::string uc = iter->first;
            std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
            std::string xml_path = save_dir + "/" + uc + ".xml";
            ucd->save_to_voc_xml_with_assign_uc(xml_path, img_path, uc);
            iter ++;
        }
    }
    bar.finish();
    delete ucd;
}

void UCDatasetUtil::parse_yolo_train_data(std::string img_dir, std::string save_dir, std::string ucd_path, std::vector<std::string> label_list)
{
    if(! is_dir(img_dir))
    {
        std::cout << ERROR_COLOR << "img_dir not exists : " << img_dir << STOP_COLOR << std::endl;
        throw "img_dir not exists";
    }

    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir not exists : " << save_dir << STOP_COLOR << std::endl;
        throw "save_dir not exists";
    }

    if(! is_file(ucd_path))
    {
        std::cout << ERROR_COLOR << "ucd_path not exists : " << ucd_path << STOP_COLOR << std::endl;
        throw "ucd_path not exists";
    }

    std::set<std::string> img_suffix {".jpg", ".JPG", ".png", ".PNG"};
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);

    if(label_list.size() == 0)
    {
        if(ucd->label_used.size() == 0)
        {
            std::cout << ERROR_COLOR << "label used list is empty, do complete it !" << STOP_COLOR << std::endl;
            throw "label list is empty and label used list is empty, do complete it";
        }
        else
        {
            label_list = ucd->label_used;
        }
    }

    int index = 0;
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        std::string uc = iter->first;
        // UCDatasetUtil::load_img(UCDatasetUtil::cache_img_dir, uc);
        std::string img_path = get_file_by_suffix_set(UCDatasetUtil::cache_img_dir, uc, img_suffix);
        std::string txt_path = save_dir + "/" + uc + ".txt";
        ucd->save_to_yolo_train_txt_with_assign_uc(txt_path, img_path, uc, label_list);
        std::cout << index << ", parse txt : " << uc << std::endl;
        index += 1;
        iter ++;
    }
    delete ucd;
}

void UCDatasetUtil::uc_analysis(std::string ucd_path)
{
    if(! is_ucd_path(ucd_path))
    {
        std::cout << ERROR_COLOR << "error ucd_path : " << ucd_path << STOP_COLOR << std::endl;
        throw "error ucd_path";
    }

    std::map<std::string, int>  uc_date_map;
    UCDataset *ucd = new UCDataset(ucd_path);
    ucd->parse_ucd();

    for(int i=0; i<ucd->uc_list.size(); i++)
    {
        std::string uc_head = ucd->uc_list[i].substr(0, 3);
        if(uc_date_map.count(uc_head) == 0)
        {
            uc_date_map[uc_head] = 1;
        }
        else
        {
            uc_date_map[uc_head] += 1;
        }
    }


    // uc to data
    std::map<std::string, int> comparison_table = 
                            {{"0", 0}, {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9}, 
                            {"a", 10}, {"b", 11}, {"c", 12}, {"d", 13}, {"e", 14}, {"f", 15}, {"g", 16}, {"h", 17}, {"i", 18},
                            {"j", 19}, {"k", 20}, {"m", 21}, {"n", 22}, {"p", 23}, {"q", 24}, {"r", 25}, {"s", 26}, {"t", 27},
                            {"u", 28}, {"v", 29}, {"w", 30}, {"x", 31}, {"y", 32}, {"z", 33}};

    std::map<std::string, int> year_dict = {{"A", 2019}, {"B", 2020}, {"C", 2021}, {"D", 2022}, {"E", 2023}, {"F", 2024}, {"G", 2025}, {"Z", 9999}};

    auto iter = uc_date_map.begin();
    while(iter != uc_date_map.end())
    {
        std::string uc = iter->first;
        int year = year_dict[uc.substr(0, 1)];
        int month = comparison_table[uc.substr(1, 1)] - 9 ;
        int day = comparison_table[uc.substr(2, 1)] - 9 ;
        if(month > 12)
        {
            month -= 12;
            day += 15;
        }
        std::string date = std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day);
        std::cout << uc << "   " << std::setw(10) << std::left << date << "   " << iter->second << std::endl;
        iter++;
    }
}

void UCDatasetUtil::conf_analysis(std::string ucd_path, int seg_count)
{
    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    std::vector<float> conf_vector;

    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            conf_vector.push_back(iter->second[i]->conf);
        }
        iter++;
    }

    // 排序
    std::sort(conf_vector.begin(), conf_vector.end());

    if(conf_vector.size() < 10)
    {
        std::cout << ERROR_COLOR << "数据量太小，没必要分析，自己打开文件去看吧" << STOP_COLOR << std::endl;
        return; 
    }

    // 按照百分位数进行分析
    int conf_size = conf_vector.size();
    std::map<float, int> conf_map;
    
    std::cout << "------------------------------" << std::endl;

    for(int i=0; i<seg_count; i++)
    {
        std::string seg_index = std::to_string(i) + "/" + std::to_string(seg_count);
        std::cout << std::setw(10) << std::left << seg_index << conf_vector[(conf_size/seg_count)*i] << std::endl; 
    }
    

    std::cout << "------------------------------" << std::endl;

    for(int i=0; i<conf_size; i++)
    {
        int conf = (int)(conf_vector[i] * seg_count);
        if(conf_map.count(conf) == 0)
        {
            conf_map[conf] = 1;
        }
        else
        {
            conf_map[conf] += 1;
        }
    }

    // 按照绝对值进行分析
    for(int i=0; i<seg_count; i++)
    {
        float conf_value = i/(seg_count * 1.0);
        std::cout << std::setw(10) << std::left << conf_value << conf_map[i] << std::endl;
    }

    std::cout << "------------------------------" << std::endl;

    delete ucd;
}

void UCDatasetUtil::area_analysis(std::string ucd_path, int seg_count)
{
    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    std::vector<float> area_vector;

    float area_max = 0;
    float area_min = std::pow(10, 8);
    auto iter = ucd->object_info.begin();
    while(iter != ucd->object_info.end())
    {
        for(int i=0; i<iter->second.size(); i++)
        {
            float each_area = iter->second[i]->get_area();
            area_vector.push_back(each_area);

            if(each_area > area_max)
            {
                area_max = each_area;
            }
            
            if(each_area < area_min)
            {
                area_min = each_area;
            }
        }
        iter++;
    }

    // 排序
    std::sort(area_vector.begin(), area_vector.end());

    if(area_vector.size() < seg_count)
    {
        std::cout << ERROR_COLOR << "数据量太小，没必要分析，自己打开文件去看吧" << STOP_COLOR << std::endl;
        return; 
    }

    // 按照百分位数进行分析
    int area_size = area_vector.size();
    std::map<int, int> area_map;
    
    std::cout << "------------------------------" << std::endl;
    std::cout << "area_min" << "     " << area_min << std::endl;
    std::cout << "area_max" << "     " << area_max << std::endl;
    std::cout << "------------------------------" << std::endl;

    for(int i=0; i<seg_count; i++)
    {
        std::string seg_index = std::to_string(i) + "/" + std::to_string(seg_count);
        std::cout << std::setw(10) << std::left << seg_index << area_vector[(area_size/seg_count)*i] << std::endl; 
    }
    
    std::cout << "------------------------------" << std::endl;

    float area_seg = (area_max - area_min);

    for(int i=0; i<area_size; i++)
    {
        int area = (int)seg_count * ((area_vector[i] - area_min) / (area_seg * 1.0));
        if(area_map.count(area) == 0)
        {
            area_map[area] = 1;
        }
        else
        {
            area_map[area] += 1;
        }
    }

    // 按照绝对值进行分析
    for(int i=0; i<seg_count; i++)
    {
        float conf_value = i/(seg_count * 1.0);
        std::cout << std::setw(10) << std::left << conf_value << area_map[i] << std::endl;
    }

    std::cout << "------------------------------" << std::endl;
    delete ucd;
}

void UCDatasetUtil::cache_clean(std::string clean_folder)
{
    if(! is_dir(clean_folder))
    {
        std::cout << "ucd path not exists" << std::endl;
    }

    // 清空全部缓存
    std::set<std::string> suffix {".jpg", ".JPG", ".png", ".PNG"};
    std::vector<std::string> all_img_path_vector = get_all_file_path(clean_folder, suffix);

    tqdm bar;
    int N = all_img_path_vector.size();
    int remove_count = 0;
    for(int i=0; i<all_img_path_vector.size(); i++)
    {
        int file_size = get_file_size(all_img_path_vector[i]);
        if(file_size == 0)
        {
            remove(all_img_path_vector[i].c_str());
            remove_count += 1;
        }
        bar.progress(i, N);
    }
    bar.finish();
    std::cout << "remove file count : " << remove_count << std::endl;
}

void UCDatasetUtil::set_fack_uc(std::string fake_folder)
{

    if(! is_dir(fake_folder))
    {
        std::cout << ERROR_COLOR << "fake foler not exists : " << fake_folder << STOP_COLOR << std::endl;
        return;
    }

    std::set<std::string> suffix = {".jpg", ".png", ".JPG", ".PNG", ".json", ".xml"};
    std::vector<std::string> file_path_vector = get_all_file_path(fake_folder, suffix);
    std::map< std::string, std::string > fack_dict;
    std::vector<std::string> uc_c = {"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"};

    int index = 0;
    for(int i=0; i<file_path_vector.size(); i++)
    {
        std::string fake_uc = "";
        std::string file_name = get_file_name(file_path_vector[i]);
        
        if(fack_dict.count(file_name) > 0)
        {
            continue;
        }
        
        index += 1;
        std::string index_str = std::to_string(index);

        std::cout << index_str << std::endl;

        if(index_str.size() == 1)
        {
            fake_uc = "Fuc000" + index_str;
        }
        else if(index_str.size() == 2)
        {
            fake_uc = "Fuc00" + index_str;
        }
        else if(index_str.size() == 3)
        {
            fake_uc = "Fuc0" + index_str;
        }
        else if(index_str.size() == 4)
        {
            fake_uc = "Fuc" + index_str;
        }
        else
        {
            std::cout << "Fake uc count must less then 10000 : " << index_str << std::endl;
            return;
        }
        fack_dict[file_name] = fake_uc;
        
        std::cout << fake_uc << std::endl;

    }

    for(int i=0; i<file_path_vector.size(); i++)
    {
        std::string file_name = get_file_name(file_path_vector[i]);
        std::string fake_uc = fack_dict[file_name];
        std::string suffix = get_file_suffix(file_path_vector[i]);

        std::string new_file_path = fake_folder + "/" + fake_uc + suffix; 

        std::cout << file_path_vector[i] << " -> " << new_file_path << std::endl;

        rename(file_path_vector[i].c_str(), new_file_path.c_str());
    }
}

void UCDatasetUtil::draw_res(std::string ucd_path, std::string save_dir, std::vector<std::string> uc_list)
{

    if(! is_file(ucd_path))
    {
        std::cout << ERROR_COLOR << "ucd path not exist : " << ucd_path << STOP_COLOR << std::endl;
        return;
    }

    if(! is_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir not exist : " << save_dir << STOP_COLOR << std::endl;
        return;
    }
    else if(access(save_dir.c_str(), 2) != 0)
    {
        std::cout << ERROR_COLOR << "save_dir don't have write access : " << save_dir << STOP_COLOR << std::endl;
    }

    UCDataset * ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);

    // 标签和颜色之间的对应关系，还可以设置默认的颜色和随机的颜色
    // 维护一个 color_dict 文件
    std::map< std::string, Color > color_map;

    if(is_file(UCDatasetUtil::color_file))
    {
        color_map = read_color_map(UCDatasetUtil::color_file);
    }

    std::cout << WARNNING_COLOR << "you can assign tag color by edit color.txt : " << color_file << STOP_COLOR << std::endl;
    std::cout << WARNNING_COLOR << "you can assign tag random color : ucd random_color ucd_path" << STOP_COLOR << std::endl;

    if(uc_list.size() == 0)
    {
        uc_list = ucd->uc_list;
    }

    tqdm bar;
    int N = uc_list.size();

    for(int i=0; i<uc_list.size(); i++)
    {
        std::string uc = ucd->uc_list[i];
        UCDatasetUtil::load_img_with_assign_uc(UCDatasetUtil::cache_img_dir, uc);
        DeteRes* dete_res = new DeteRes();
        ucd->get_dete_res_with_assign_uc(dete_res, uc);
        std::string img_path = UCDatasetUtil::cache_img_dir + "/" + uc + ".jpg";
        dete_res->parse_img_info(img_path);
        std::string save_path = save_dir + "/" + uc + ".jpg";

        if(! is_file(save_path))
        {
            dete_res->draw_dete_res(save_path, color_map);
        }

        bar.progress(i, N);
        delete dete_res;
    }
    bar.finish();
    delete ucd;
}

void UCDatasetUtil::get_random_color_map(std::string ucd_path)
{

    UCDataset* ucd = new UCDataset(ucd_path);
    ucd->parse_ucd(true);
    
    std::map< std::string, Color > color_map;
    if(is_file(UCDatasetUtil::color_file))
    {
        color_map = read_color_map(UCDatasetUtil::color_file);
    }

    // random color for 
    std::set<std::string> tags = ucd->get_tags();
    auto iter_tag = tags.begin();
    while(iter_tag != tags.end())
    {
        if(color_map.count(iter_tag->data()) == 0)
        {
            Color color;
            // rgb 最小值是 20 防止颜色太黑，看不清楚
            color.r = std::rand() % 200 + 55;
            color.g = std::rand() % 200 + 55;
            color.b = std::rand() % 200 + 55;
            color_map[iter_tag->data()] = color;
        }
        iter_tag++;
    }
    // save color 
    write_color_map(color_map, UCDatasetUtil::color_file);

    std::cout << WARNNING_COLOR << "color file : " << UCDatasetUtil::color_file << STOP_COLOR << std::endl;
}

void UCDatasetUtil::list_uci(std::string folder_path)
{

    if(! is_read_dir(folder_path))
    {
        std::cout << ERROR_COLOR << "folder is not readable : " << folder_path << STOP_COLOR << std::endl;
        return;
    }

    std::vector<std::string> file_path  = get_all_file_path(folder_path);
    std::set<std::string> suffix {".uci"};
    std::vector<std::string> ucd_path   = filter_by_suffix(file_path, suffix);
    std::map< std::string,  std::map<std::string, int> > ucd_info;

    for(int i=0; i<ucd_path.size(); i++)
    {
        std::string ucd_name = get_file_name(ucd_path[i]);
        ucd_info[ucd_name]["uci_count"]     = 1;
        ucd_info[ucd_name]["obi_count"]     = 1;
        ucd_info[ucd_name]["szi_count"]     = 1;
        ucd_info[ucd_name]["volume_size"]   = -1;
        
        // 读取 uci 文件
        UCDataset* ucd = new UCDataset();
        ucd->load_uci(ucd_path[i]);
        ucd->parse_volume(0, false, false);
        ucd_info[ucd_name]["volume_size"] = ucd->volume_size;

        int index = 1;
        while(true)
        {
            std::string volume_uci_path = folder_path + "/" + ucd_name + ".u" + std::to_string(index);
            std::string volume_obi_path = folder_path + "/" + ucd_name + ".o" + std::to_string(index);
            std::string volume_szi_path = folder_path + "/" + ucd_name + ".s" + std::to_string(index);
            if(is_file(volume_uci_path))
            {
                index += 1;
                ucd_info[ucd_name]["uci_count"] += 1;
                if(is_file(volume_obi_path))
                {
                    ucd_info[ucd_name]["obi_count"] += 1;
                }
                if(is_file(volume_szi_path))
                {
                    ucd_info[ucd_name]["szi_count"] += 1;
                }
            }
            else
            {
                break;
            }
        }
    }
    //

    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(20) << "uci_name"  << std::left << std::setfill(' ') << std::setw(15) << "uci_count" << std::left << std::setfill(' ') << std::setw(15) << std::left << "obi_count" << std::left << std::setfill(' ') << std::setw(15) << std::left << "volume_size" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    auto iter_1 = ucd_info.begin();
    while(iter_1 != ucd_info.end())
    {
        std::cout << std::left << std::setfill(' ') << std::setw(20) << iter_1->first << std::left << std::setfill(' ') << std::setw(15)  << iter_1->second["uci_count"]  << std::left << std::setfill(' ') << std::setw(15) << iter_1->second["obi_count"] << std::left << std::setfill(' ') << std::setw(15) << iter_1->second["volume_size"] << std::endl;
        iter_1++;
    }
    std::cout << "-------------------------------------------------------------------" << std::endl;
}

void UCDatasetUtil::delete_uci(std::string uci_path)
{
    // 找到 uci 对应的所有分卷，直接删除

    if(! UCDatasetUtil::is_uci_path(uci_path))
    {
        std::cout << ERROR_COLOR << "not uci path : " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset();
    ucd->load_uci(uci_path);

    for(int i=0; i<ucd->volume_count; i++)
    {
        std::string uci_path = ucd->get_uci_path(i);
        std::string obi_path = ucd->get_obi_path(i);
        std::string szi_path = ucd->get_szi_path(i);

        if(is_file(uci_path))
        {
            remove(uci_path.c_str());
        }
        if(is_file(obi_path))
        {
            remove(obi_path.c_str());
        }
        if(is_file(szi_path))
        {
            remove(szi_path.c_str());
        }
    }
    delete ucd;
}

void UCDatasetUtil::copy_uci(std::string uci_path, std::string save_path)
{
    if(! UCDatasetUtil::is_uci_path(uci_path))
    {
        std::cout << ERROR_COLOR << "not uci path : " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    std::string save_dir = get_file_folder(save_path);
    std::string save_name = get_file_name(save_path);

    if(! is_write_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir is not writeable : " << STOP_COLOR << save_dir << std::endl;
        return;
    }

    if(! pystring::endswith(save_path, ".uci"))
    {
        std::cout << ERROR_COLOR << "save_path not end with .uci : " << STOP_COLOR << save_path << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset();
    ucd->load_uci(uci_path);

    for(int i=0; i<ucd->volume_count; i++)
    {
        std::string uci_path = ucd->get_uci_path(i);
        std::string obi_path = ucd->get_obi_path(i);
        std::string szi_path = ucd->get_szi_path(i);
        std::string uci_suffix = get_file_suffix(uci_path);
        std::string obi_suffix = get_file_suffix(obi_path);
        std::string szi_suffix = get_file_suffix(szi_path);
        std::string save_uci_path = save_dir + "/" + save_name + uci_suffix; 
        std::string save_obi_path = save_dir + "/" + save_name + obi_suffix; 
        std::string save_szi_path = save_dir + "/" + save_name + szi_suffix; 

        if(is_file(uci_path))
        {
            copy_file(uci_path, save_uci_path);
        }
        if(is_file(obi_path))
        {
            copy_file(obi_path, save_obi_path);
        }
        if(is_file(szi_path))
        {
            copy_file(szi_path, save_szi_path);
        }
    }
    delete ucd;
}

void UCDatasetUtil::move_uci(std::string uci_path, std::string save_path)
{
    if(! UCDatasetUtil::is_uci_path(uci_path))
    {
        std::cout << ERROR_COLOR << "not uci path : " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    std::string save_dir = get_file_folder(save_path);
    std::string save_name = get_file_name(save_path);

    if(! is_write_dir(save_dir))
    {
        std::cout << ERROR_COLOR << "save_dir is not writeable : " << STOP_COLOR << save_dir << std::endl;
        return;
    }

    if(! pystring::endswith(save_path, ".uci"))
    {
        std::cout << ERROR_COLOR << "save_path not end with .uci : " << STOP_COLOR << save_path << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset();
    ucd->load_uci(uci_path);

    for(int i=0; i<ucd->volume_count; i++)
    {
        std::string uci_path = ucd->get_uci_path(i);
        std::string obi_path = ucd->get_obi_path(i);
        std::string szi_path = ucd->get_szi_path(i);
        std::string uci_suffix = get_file_suffix(uci_path);
        std::string obi_suffix = get_file_suffix(obi_path);
        std::string szi_suffix = get_file_suffix(szi_path);
        std::string save_uci_path = save_dir + "/" + save_name + uci_suffix; 
        std::string save_obi_path = save_dir + "/" + save_name + obi_suffix; 
        std::string save_szi_path = save_dir + "/" + save_name + szi_suffix; 

        if(is_file(uci_path))
        {
            rename(uci_path.c_str(), save_uci_path.c_str());
        }
        if(is_file(obi_path))
        {
            rename(obi_path.c_str(), save_obi_path.c_str());
        }
        if(is_file(szi_path))
        {
            rename(szi_path.c_str(), save_szi_path.c_str());
        }
    }
    delete ucd;
}

void UCDatasetUtil::json_to_uci(std::string json_path, std::string uci_path, int volume_size)
{
    if(! UCDatasetUtil::is_ucd_path(json_path))
    {
        std::cout << ERROR_COLOR << "json path is not legal : " << json_path << STOP_COLOR << std::endl;
        return;
    }
 
    if(! pystring::endswith(uci_path, ".uci"))
    {
        std::cout << ERROR_COLOR << "uci path is not legal : " << uci_path << STOP_COLOR << std::endl;
        return;
    }

    UCDataset* ucd = new UCDataset(json_path);
    ucd->parse_ucd(true);
    ucd->to_uci(uci_path, volume_size);
    delete ucd;
}

void UCDatasetUtil::uci_to_json(std::string uci_path, std::string json_path)
{

}

void UCDatasetUtil::interset_ucds(std::string save_path, std::string ucd_path_a, std::string ucd_path_b)
{
    UCDataset* ucd_a = new UCDataset(ucd_path_a);
    UCDataset* ucd_b = new UCDataset(ucd_path_b);
    UCDataset* ucd_res = new UCDataset(save_path);

    ucd_a->parse_ucd(false);
    ucd_b->parse_ucd(false);

    std::vector<std::string> uc_intersection;
    std::set<std::string> uc_set1(ucd_a->uc_list.begin(), ucd_a->uc_list.end());
    std::set<std::string> uc_set2(ucd_b->uc_list.begin(), ucd_b->uc_list.end());
    std::set_intersection(uc_set1.begin(), uc_set1.end(), uc_set2.begin(), uc_set2.end(), std::inserter(uc_intersection, uc_intersection.begin()));

    ucd_res->uc_list = uc_intersection;
    ucd_res->save_to_ucd(save_path);
}

void UCDatasetUtil::filter_by_tags_volume(std::set<std::string> tags, std::string uci_path,  std::string save_dir, std::string save_name, int volume_size)
{

    UCDataset* ucd = new UCDataset();
    ucd->load_uci(uci_path);
    UCDataset* res = new UCDataset();
    
    tqdm bar;
    int N = ucd->volume_count;

    int volume_index = 0;
    for(int i=0; i<ucd->volume_count; i++)
    {
        bar.progress(i, N);
        ucd->parse_volume(i, true, true, false);
        ucd->filter_by_tags(tags);
        res->add_ucd_info(ucd);

        if(res->get_info_count() > volume_size * 1000 * 100)
        {
            res->volume_size = volume_size;
            res->save_to_huge_ucd(save_dir, save_name, volume_index);
            volume_index += 1;
            // clear each_ucd 
            res->clear_obj_info();
            res->object_info.clear();
            res->uc_list.clear();
            res->size_info.clear();
        }
    }
    bar.finish();

    // rest
    if(res->get_info_count() > 0)
    {
        res->volume_size = volume_size;
        res->save_to_huge_ucd(save_dir, save_name, volume_index);
    }
    delete res;
    delete ucd;
}








