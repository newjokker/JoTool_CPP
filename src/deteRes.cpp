
#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <typeinfo>
#include <string>
#include <cstdlib>
#include "../include/deteRes.hpp"
#include "../include/tinyxml2.h"
#include "../include/fileOperateUtil.hpp"

namespace jotools
{

DeteRes::DeteRes(std::string xml_path, std::string img_path)
{   
    // init 
    DeteRes::width = -1;
    DeteRes::height = -1;
    DeteRes::depth = -1;


    if(xml_path != "")
    {
        DeteRes::parse_xml_info(xml_path);
    }

    if(img_path != "")
    {
        DeteRes::parse_img_info(img_path);
    }
}

int DeteRes::add_dete_obj(int x1, int y1, int x2, int y2, float conf, std::string tag)
{
    DeteObj a;

    a.x1 = x1;
    a.y1 = y1;
    a.x2 = x2;
    a.y2 = y2;
    a.conf = conf;
    a.tag = tag;

    DeteRes::add_dete_obj(a);
}

int DeteRes::add_dete_obj(DeteObj dete_obj)
{
    DeteRes::alarms.push_back(dete_obj);
    return 1;
}

void DeteRes::print_format()
{
    std::cout << "---------------------------------------" << std::endl;

    std::cout << "file name :" << DeteRes::file_name << std::endl;
    std::cout << "img path:" << DeteRes::img_path << std::endl;
    std::cout << "folder :" << DeteRes::folder << std::endl;
    std::printf("(h,w,d) : (%d, %d, %d) \n", DeteRes::height, DeteRes::width, DeteRes::depth); 
    // "width-height-depth : " << DeteRes::width << "-" << DeteRes::height << "-" << DeteRes::depth << std::endl;
    std::cout << "----------------------" << std::endl;

    for(int i=0; i<DeteRes::alarms.size(); i++)
    {
        DeteRes::alarms[i].print_format();
    }
    std::cout << "---------------------------------------" << std::endl;
}

int DeteRes::del_dete_obj(DeteObj dete_obj)
{
    for(int i=0; i<DeteRes::alarms.size();i++)
    {
        if(DeteRes::alarms[i] == dete_obj)
        {
            DeteRes::alarms.erase(DeteRes::alarms.begin() + i);
            return 1;
        }
    }
}

int DeteRes::filter_by_tags(std::vector<std::string> tags)
{
    std::vector<DeteObj> alarms;
    for(int i=0; i<DeteRes::alarms.size();i++)
    {
        bool keep_dete_obj = false; 
        for(int j=0; j<tags.size(); j++)
        {
            if(tags[j] == DeteRes::alarms[i].tag)
            {
                keep_dete_obj = true;
                break;
            }
        }
        if(keep_dete_obj)
        {
            alarms.push_back(DeteRes::alarms[i]);
        }
    }
    DeteRes::alarms = alarms;
    return 1;
}

bool DeteRes::has_dete_obj(DeteObj dete_obj)
{
    for(int i=0; i<DeteRes::alarms.size(); i++)
    {
        if(DeteRes::alarms[i] == dete_obj)
        {
            return true;
        }
    }
    return false;
}

bool DeteRes::operator+(const DeteRes other)
{

    // todo 这边写错了应该，非成员函数，每次要传入两个参数，这边只是传入了一个
    // refer : https://www.runoob.com/cplusplus/cpp-overloading.html

    // fixme 这边返回的应该是一个新的 DeteRes 
    for(int i=0; i<other.alarms.size(); i++)
    {
        if (DeteRes::has_dete_obj(other.alarms[i]) == false)
        {
            DeteRes::add_dete_obj(other.alarms[i]);
        }
    }
}

DeteObj& DeteRes::operator[](const int i)
{
    // 为什么没用，不知道是什么原因
    return DeteRes::alarms[i];
}

int DeteRes::size()
{
    return this->alarms.size();
}

DeteRes DeteRes::update(DeteRes other)
{
    for(int i=0; i<other.alarms.size(); i++)
    {
        if (DeteRes::has_dete_obj(other.alarms[i]) == false)
        {
            DeteRes::add_dete_obj(other.alarms[i]);
        }
    }
    return *this;
}

bool DeteRes::parse_xml_info(const std::string xml_path)
{

    try
    {
        // todo 判断文件路径是否存在
        if(is_file(xml_path) == false)
        {
            std::cout << "xml path not exists : " << xml_path << std::endl;
            throw "parse xml error, xml path not exists, " + xml_path;           
        }

        tinyxml2::XMLDocument doc;
        doc.LoadFile( xml_path.c_str());
        tinyxml2::XMLElement* root = doc.RootElement();

        if(root == NULL)
        {
            std::cout << "parse xml error, lose height, " + xml_path << std::endl;
            throw "parse xml error, lose height, " + xml_path;           
        }

        tinyxml2::XMLElement* objects = root->FirstChildElement("object");
        tinyxml2::XMLElement* img_size = root->FirstChildElement("size");    

        if(img_size)
        {
            // height
            auto height = img_size->FirstChildElement("height");
            if(height)
            {
                DeteRes::height = std::stoi(height->GetText());
            }
            else
            {
                std::cout << "parse xml error, lose height, " + xml_path << std::endl;
                throw "parse xml error, lose height, " + xml_path;
            }

            // width
            auto width = img_size->FirstChildElement("width");
            if(height)
            {
                DeteRes::width = std::stoi(width->GetText());
            }
            else
            {
                std::cout << "parse xml error, lose width, " + xml_path << std::endl;
                throw "parse xml error, lose width, " + xml_path;
            }

            // depth
            auto depth = img_size->FirstChildElement("depth");
            if(depth)
            {
                DeteRes::depth = std::stoi(depth->GetText());
            }
            else
            {
                std::cout << "parse xml error, lose depth, " + xml_path << std::endl;
                throw "parse xml error, lose depth, " + xml_path;
            }
        }

        // folder
        auto folder = root->FirstChildElement("folder");   
        if(folder)
        {
            auto folder_str = folder->GetText();
            if(folder_str)
            {
                DeteRes::folder = folder_str;
            }
            else
            {
                DeteRes::folder = "";
            }

        }
        else
        {
            std::cout << "parse xml error, lose folder, " + xml_path << std::endl;
            throw "parse xml error, lose folder, " + xml_path;
        }

        // path
        auto path = root->FirstChildElement("path");   
        if(path)
        {
            auto img_path_str = path->GetText();
            if(img_path_str)
            {
                DeteRes::img_path = img_path_str;
            }
            else
            {
                DeteRes::img_path = "";
            }
        }
        else
        {
            std::cout << "parse xml error, lose path, " + xml_path << std::endl;
            throw "parse xml error, lose path, " + xml_path;
        }

        // filename
        auto file_name = root->FirstChildElement("filename");   
        if(file_name)
        {
            auto file_name_str = file_name->GetText();
            if(file_name_str)
            {
                DeteRes::file_name = file_name_str;
            }
            else
            {
                DeteRes::file_name = "";
            }
        }
        else
        {
            std::cout << "parse xml error, lose filename, " + xml_path << std::endl;
            throw "parse xml error, lose filename, " + xml_path;          
        }

        // object info
        if(objects)
        {
            while(objects){
                DeteObj* obj_info = new DeteObj();

                // name
                auto name = objects->FirstChildElement("name");
                if(name)
                {
                    obj_info->tag = name->GetText();
                }
                else
                {
                    std::cout << "parse xml error, lose tag, " + xml_path << std::endl;
                    throw "parse xml error, lose tag, " + xml_path;
                }

                // prob
                auto prob = objects->FirstChildElement("prob");
                if(prob)
                {
                    obj_info->conf = std::stof(prob->GetText());
                }
                else
                {
                    obj_info->conf = -1;
                }

                // bndbox
                tinyxml2::XMLElement* bndbox = objects->FirstChildElement("bndbox");
                if(bndbox)
                {
                    // xmin
                    auto xmin = bndbox->FirstChildElement("xmin");
                    if(xmin)
                    {
                        obj_info->x1 = std::stof(xmin->GetText());
                    }
                    else
                    {
                        std::cout << "parse xml error, lose xmin, " + xml_path << std::endl;
                        throw "parse xml error, lose xmin" + xml_path;
                    }
                    // ymin
                    auto ymin = bndbox->FirstChildElement("ymin");
                    if(ymin)
                    {
                        obj_info->y1 = std::stof(ymin->GetText());
                    }
                    else
                    {
                        std::cout << "parse xml error, lose ymin, " + xml_path << std::endl;
                        throw "parse xml error, lose ymin" + xml_path;
                    }
                    // xmax
                    auto xmax = bndbox->FirstChildElement("xmax");
                    if(xmax)
                    {
                        obj_info->x2 = std::stof(xmax->GetText());
                    }
                    else
                    {
                        std::cout << "parse xml error, lose xmax, " + xml_path << std::endl;
                        throw "parse xml error, lose xmax" + xml_path;
                    }
                    // ymax
                    auto ymax = bndbox->FirstChildElement("ymax");
                    if(ymax)
                    {
                        obj_info->y2 = std::stof(ymax->GetText());
                    }
                    else
                    {
                        std::cout << "parse xml error, lose ymax, " + xml_path << std::endl;
                        throw "parse xml error, lose ymax" + xml_path;
                    }

                    DeteRes::alarms.push_back(*obj_info);
                }
                else
                {
                    std::cout << "parse xml error, lose bndbox, " + xml_path << std::endl;
                    throw "parse xml error, lose bndbox" + xml_path;
                }
                objects = objects->NextSiblingElement();
                delete obj_info;
            }
        }
        
        return true;
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
        throw "parse xml error, " + xml_path;
        return false;
    }
    return false;
}

bool DeteRes::parse_img_info(std::string img_path)
{
    // mat
    cv::Mat image = cv::imread(img_path);
    DeteRes::img_ndarry = image; 
    // path
    FilePath file_path = parse_file_path(img_path);
    DeteRes::file_name = file_path.name_suffix;
    DeteRes::folder = file_path.folder;
    DeteRes::img_path = img_path;
    // size
    DeteRes::height = image.size().height;
    DeteRes::width = image.size().width;
    DeteRes::depth = 3;
}

int DeteRes::save_to_xml(std::string save_path)
{
    
    // refer : https://blog.csdn.net/K346K346/article/details/48750417

    // 声明
    const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
    tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
    doc->Parse(declaration);

    // root element
    tinyxml2::XMLElement* root = doc->NewElement("annotation");
    doc->InsertEndChild(root);

    // insert attr
    // file name
    tinyxml2::XMLElement* file_name = doc->NewElement("filename");
    tinyxml2::XMLText* file_name_text = doc->NewText(DeteRes::file_name.c_str());
    file_name->InsertEndChild(file_name_text);
    root->InsertEndChild(file_name);
    // path
    tinyxml2::XMLElement* path = doc->NewElement("path");
    tinyxml2::XMLText* path_text = doc->NewText(DeteRes::img_path.c_str());
    path->InsertEndChild(path_text);
    root->InsertEndChild(path);
    // folder
    tinyxml2::XMLElement* folder = doc->NewElement("folder");
    tinyxml2::XMLText* folder_text = doc->NewText(DeteRes::folder.c_str());
    folder->InsertEndChild(folder_text);
    root->InsertEndChild(folder);

    // size
    tinyxml2::XMLElement* size = doc->NewElement("size");
    root->InsertEndChild(size);
    // height
    tinyxml2::XMLElement* height = doc->NewElement("height");
    tinyxml2::XMLText* height_text = doc->NewText(std::to_string(DeteRes::height).c_str());
    height->InsertEndChild(height_text);
    size->InsertEndChild(height);
    // width
    tinyxml2::XMLElement* width = doc->NewElement("width");
    tinyxml2::XMLText* width_text = doc->NewText(std::to_string(DeteRes::width).c_str());
    width->InsertEndChild(width_text);
    size->InsertEndChild(width);
    // depth
    tinyxml2::XMLElement* depth = doc->NewElement("depth");
    tinyxml2::XMLText* depth_text = doc->NewText(std::to_string(DeteRes::depth).c_str());
    depth->InsertEndChild(depth_text);
    size->InsertEndChild(depth);

    for(int i=0; i<DeteRes::alarms.size();i++)
    {
        tinyxml2::XMLElement* object = doc->NewElement("object");
        root->InsertEndChild(object);

        // tag
        tinyxml2::XMLElement* tag = doc->NewElement("name");
        tinyxml2::XMLText* tag_text = doc->NewText(DeteRes::alarms[i].tag.c_str());
        tag->InsertEndChild(tag_text);
        object->InsertEndChild(tag);
        // name
        tinyxml2::XMLElement* conf = doc->NewElement("prob");
        tinyxml2::XMLText* conf_text = doc->NewText(std::to_string(DeteRes::alarms[i].conf).c_str());
        conf->InsertEndChild(conf_text);
        object->InsertEndChild(conf);
        // bndbox
        tinyxml2::XMLElement* bndbox = doc->NewElement("bndbox");
        object->InsertEndChild(bndbox);
        // xmin
        tinyxml2::XMLElement* xmin = doc->NewElement("xmin");
        tinyxml2::XMLText* xmin_text = doc->NewText(std::to_string(DeteRes::alarms[i].x1).c_str());
        xmin->InsertEndChild(xmin_text);
        bndbox->InsertEndChild(xmin);
        // xmax
        tinyxml2::XMLElement* xmax = doc->NewElement("xmax");
        tinyxml2::XMLText* xmax_text = doc->NewText(std::to_string(DeteRes::alarms[i].x2).c_str());
        xmax->InsertEndChild(xmax_text);
        bndbox->InsertEndChild(xmax);
        // ymin
        tinyxml2::XMLElement* ymin = doc->NewElement("ymin");
        tinyxml2::XMLText* ymin_text = doc->NewText(std::to_string(DeteRes::alarms[i].y1).c_str());
        ymin->InsertEndChild(ymin_text);
        bndbox->InsertEndChild(ymin);
        // ymax
        tinyxml2::XMLElement* ymax = doc->NewElement("ymax");
        tinyxml2::XMLText* ymax_text = doc->NewText(std::to_string(DeteRes::alarms[i].y2).c_str());
        ymax->InsertEndChild(ymax_text);
        bndbox->InsertEndChild(ymax);
    }
    // save
    doc->SaveFile(save_path.c_str());
    return true;
}

std::map<std::string, int> DeteRes::count_tags()
{
    std::map <std::string, int> count_res;
    for(int i=0; i<DeteRes::alarms.size(); i++)
    {
        std::string tag = DeteRes::alarms[i].tag;
        if(count_res.count(tag) == 0)
        {
            count_res[tag] = 1;
        }
        else
        {
            count_res[tag] += 1;
        }
    }
    // // 打印出结果
    // auto iter = count_res.begin();
    // while(iter != count_res.end()) {
    //     std::cout << iter->first << " : " << iter->second << std::endl;
    //     iter++;
    // }
    return count_res;
}

cv::Mat DeteRes::get_sub_img_by_dete_obj(DeteObj dete_obj, bool RGB)
{
    // todo 加上 RGB 的转换操作 
    cv::Rect rect(dete_obj.x1, dete_obj.y1, dete_obj.x2-dete_obj.x1, dete_obj.y2-dete_obj.y1); 
    cv::Mat ROI = DeteRes::img_ndarry(rect); 
    return ROI;
}

int DeteRes::crop_dete_obj(std::string save_dir, bool split_by_tag, std::string save_name)
{
    if((DeteRes::img_path) == "" && (save_name == ""))
    {
        throw "* img_path && save_name is empty !";
    }
    
    if(save_name == "")
    {
        save_name = get_file_name(DeteRes::img_path);
    }

    std::string each_save_name;
    std::string tag_dir;
    cv::Mat roi;
    for(int i=0; i<DeteRes::size(); i++)
    {
        roi = DeteRes::get_sub_img_by_dete_obj(DeteRes::alarms[i]);
        if(split_by_tag)
        {
            tag_dir = save_dir + "/" + DeteRes::alarms[i].tag;
            if(is_dir(tag_dir) == false)
            {
                create_folder(tag_dir);
            }
            each_save_name = tag_dir + "/" + save_name + "-+-" + DeteRes::alarms[i].get_name_str() + ".jpg"; 
        }
        else
        {
            each_save_name = save_dir + "/" + save_name + "-+-" + DeteRes::alarms[i].get_name_str() + ".jpg"; 
        }
        cv::imwrite(each_save_name, roi);
    }
}

bool dete_obj_greater_sort(DeteObj a, DeteObj b)
{
    if(a.conf >= b.conf)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool dete_obj_less_sort(DeteObj a, DeteObj b)
{
    if(a.conf <= b.conf)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void DeteRes::sort_by_conf(bool reverse)
{
    if(reverse == true)
    {
        // 大的放前面
        std::sort(DeteRes::alarms.begin(), DeteRes::alarms.end(), dete_obj_greater_sort);
    }
    else
    {
        // 小的放前面
        std::sort(DeteRes::alarms.begin(), DeteRes::alarms.end(), dete_obj_less_sort);
    }
}

float iou_between_obj(DeteObj a, DeteObj b, bool ignore_tag=false)
{

    if((ignore_tag == false) && (a.tag != b.tag))
    {
        return false;
    }

    int x1 = a.x1;
    int y1 = a.y1;
    int h1 = a.y2 - a.y1;
    int w1 = a.x2 - a.x1;
    int x2 = b.x1;
    int y2 = b.y1;
    int h2 = b.y2 - b.y1;
    int w2 = b.x2 - b.x1;

    int endx = std::max(x1 + w1, x2 + w2);
    int startx = std::min(x1, x2);
    int width = w1 + w2 - (endx - startx);
    int endy = std::max(y1 + h1, y2 + h2);
    int starty = std::min(y1, y2);
    int height = h1 + h2 - (endy - starty);
    if (width > 0 && height > 0) {
        int area = width * height;
        int area1 = w1 * h1;
        int area2 = w2 * h2;
        float ratio = (float)area / (area1 + area2 - area);
        return ratio;
    } else {
        return 0.0;
    }
}

void DeteRes::do_nms(float threshold, bool ignore_tag)
{
    // 从大到小排列，结果不进行反转
    DeteRes::sort_by_conf(false);
    std::vector<DeteObj> alarms_origin = DeteRes::alarms;
    std::vector<DeteObj> alarms_res;

    // 
    if(alarms_origin.size() > 0)
    {
        alarms_res.push_back(alarms_origin.back());
        alarms_origin.pop_back();
    }
    else
    {
        return ;
    }

    // 
    while(alarms_origin.size() > 0)
    {
        DeteObj each_obj = alarms_origin.back();
        alarms_origin.pop_back();
        //
        bool is_add = true;
        for(int i=0; i<alarms_res.size(); i++)
        {
            float each_iou = iou_between_obj(alarms_res[i], each_obj, ignore_tag);
            if(each_iou > threshold)
            {
                is_add = false;
                break;
            }
        }

        if(is_add)
        {
            alarms_res.push_back(each_obj);
        }
    }
    DeteRes::alarms = alarms_res;

}



}
