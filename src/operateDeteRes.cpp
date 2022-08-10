

#include <iostream>
#include <vector>
#include <map>
#include "../include/deteRes.hpp"
#include "../include/fileOperateUtil.hpp"
#include "../include/pystring.h"
#include "../include/operateDeteRes.hpp"
#include <stdio.h>
#include <sys/stat.h> 
// #include "./parse_img_exif_info.hpp"
#include "include/easyexif.h"
#include "include/imageinfo.hpp"

namespace jotools
{


std::map<std::string, int> count_tags(std::string floder_path)
{
    std::map<std::string, int> count_res;
    std::vector<std::string> file_names = get_all_file_path(floder_path);
    std::set<std::string> suffixs;
    suffixs.insert(".xml");
    std::vector<std::string> file_name_xml = filter_by_suffix(file_names, suffixs);

    // 
    for(int i; i<file_name_xml.size(); i++)
    {
        DeteRes *dete_res = new DeteRes(file_name_xml[i]);
        std::map<std::string, int> each_res = dete_res->count_tags();
        std::map<std::string, int>::iterator iter;

        iter = each_res.begin();
        while(iter != each_res.end()) 
        {
            if(count_res.count(iter->first) == 0)
            {
                count_res[iter->first] = iter->second;
            }
            else
            {
                count_res[iter->first] += iter->second;
            }
            iter++;
        }
        delete dete_res;
    }
    // print res
    std::cout << "-------------------------------" << std::endl;
    int tags_numb = 0; 
    std::map<std::string, int>::iterator iter;
    iter = count_res.begin();
    while(iter != count_res.end()) 
    {
        std::cout << iter->first << " : " << iter->second << std::endl;
        tags_numb += iter->second;
        iter++;
    }
    // extra info
    std::cout << "number of xml : " << file_name_xml.size() << std::endl;
    std::cout << "number of tag : " << tags_numb << std::endl;
    std::cout << "-------------------------------" << std::endl;
    return count_res;
}

void cut_small_img(std::string img_dir, std::string xml_dir, std::string save_dir, bool split_by_tag)
{
    std::vector<std::string> file_path_list = get_all_file_path(img_dir);

    std::vector<std::string> img_path_list;
    std::set<std::string> suffixs;
    suffixs.insert(".jpg");
    img_path_list = filter_by_suffix(file_path_list, suffixs);

    for(int i=0; i<img_path_list.size();i++)
    {
        std::string each_xml_path = xml_dir + "/" + get_file_name(img_path_list[i]) + ".xml";
        if (is_file(each_xml_path))
        {
            std::cout << i << " : " << each_xml_path << " , " << img_path_list[i] << std::endl;
            DeteRes a(each_xml_path, img_path_list[i]);
            a.crop_dete_obj(save_dir, split_by_tag, "");
        }
    }
}

void get_xml_from_crop_img(std::string crop_dir, std::string save_xml_dir)
{
    // 遍历得到小文件夹
    // 遍历每一个小文件夹下载的 jpg 和 png 图片，拿到图片名中的信息（原始文件名 + 位置信息）
    // 根据已有的信息生成 xml 


    // get crop info 

    std::map<std::string, std::vector<std::string>> xml_info_map;
    std::vector<std::string> folder_path_list = get_all_folder_path(crop_dir);

    for(int i=0; i<folder_path_list.size(); i++)
    {
        std::vector<std::string> file_path_vector = get_all_file_path(folder_path_list[i]);

        std::set<std::string> suffixs;
        suffixs.insert(".jpg");
        suffixs.insert(".JPG");
        suffixs.insert(".png");
        suffixs.insert(".PNG");

        std::vector<std::string> img_path_vector = filter_by_suffix(file_path_vector, suffixs);

        for(int j=0; j<img_path_vector.size(); j++)
        {
            std::string file_name = get_file_name(img_path_vector[j]);
            std::vector<std::string> loc_str_list = pystring::split(file_name, "-+-");
            std::string loc_str = loc_str_list[loc_str_list.size()-1];
            std::string region_name = file_name.substr(0, pystring::rfind(file_name, "-+-"));
            xml_info_map[region_name].push_back(loc_str);
        }
        // std::cout << "-----------------------" << std::endl;
    }


    // get xml
    auto iter = xml_info_map.begin();
    while (iter != xml_info_map.end())
    {
        DeteRes* dete_res = new DeteRes();
        for(int i=0; i<iter->second.size(); i++)
        {
            DeteObj* dete_obj = new DeteObj();
            dete_obj->load_from_name_str(iter->second[i]);
            dete_res->add_dete_obj(*dete_obj);
            delete dete_obj;
            // std::cout << iter->first << " : " << iter->second.size() << std::endl;
        }
        std::string save_xml_path = save_xml_dir + "/" + iter->first + ".xml"; 
        dete_res->save_to_xml(save_xml_path);
        delete dete_res;
        iter++;
    }
}

// 查看文件分布
void count_files(std::string folder_path, bool recursive)
    {
        std::map<std::string, int> file_count_map;
        std::vector<std::string> file_path_vector;
        if(recursive)
        {
            file_path_vector = get_all_file_path_recursive(folder_path);
        }
        else
        {
            file_path_vector = get_all_file_path(folder_path);
        }

        for(int i=0; i<file_path_vector.size(); i++)
        {

            std::string suffix = get_file_suffix(file_path_vector[i]);

            if(file_count_map.count(suffix) == 0)
            {
                file_count_map[suffix] = 1;
            }
            else
            {
                file_count_map[suffix] +=1;
            }
        }

        auto iter = file_count_map.begin();
        while(iter != file_count_map.end())
        {
            std::cout << iter->first << " : " << iter->second << std::endl;
            iter++;
        }
    }

// 检查 训练的 xml 和 img 是否对应
void xml_check(std::string xml_dir, std::string img_dir, int size_th, bool remove_error_path)
{

    // if folder exists
    if(! is_dir(xml_dir))
    {
        std::cout << "xml dir not exist, " << xml_dir << std::endl;
        throw "xml dir not exist";
    }

    if(! is_dir(img_dir))
    {
        std::cout << "img dir not exist, " << img_dir << std::endl;
        throw "img dir not exist";
    }

    std::vector<std::string> error_file_vector;

    // suffixs
    std::set<std::string> suffixs_xml {".xml"};
    std::set<std::string> suffixs_img {".jpg", ".png", ".JPG", ".PNG"};
    
    // xml
    std::vector<std::string> all_xml_vector = get_all_file_path(xml_dir, suffixs_xml);
    for(int i=0; i<all_xml_vector.size(); i++)
    {
        std::string xml_name = get_file_name(all_xml_vector[i]);
        std::string img_path = get_file_by_suffix_set(img_dir, xml_name, suffixs_img);
        if(img_path == "")
        {
            std::cout << "extra_xml: " << all_xml_vector[i] << std::endl;
            error_file_vector.push_back(all_xml_vector[i]);
        }
    }
    
    // img
    std::vector<std::string> all_img_vector = get_all_file_path(img_dir, suffixs_img);
    std::vector<std::string> img_vector;
    for(int i=0; i<all_img_vector.size(); i++)
    {
        std::string img_name = get_file_name(all_img_vector[i]);
        std::string xml_path = get_file_by_suffix_set(xml_dir, img_name, suffixs_xml);
        if(xml_path == "")
        {
            std::cout << "extra_img: " << all_img_vector[i] << std::endl;
            error_file_vector.push_back(all_img_vector[i]);
        }
        else
        {
            img_vector.push_back(all_img_vector[i]);
        }
    }

    // compare 
    for(int i=0; i<img_vector.size(); i++)
    {
        std::string xml_path = xml_dir + "/" + get_file_name(img_vector[i]) + ".xml";
        
        DeteRes* dete_res = new DeteRes(xml_path); 

        // compare height, width
        int height, width;

        FILE *file = fopen(img_vector[i].c_str(), "rb");
        auto imageInfo = getImageInfo<IIFileReader>(file);
        fclose(file);

        height = imageInfo.getHeight();
        width =  imageInfo.getWidth();

        if(dete_res->height != height)
        {
            std::cout << "error_size: " << xml_path << std::endl;
            std::cout << "error_size: " << img_vector[i] << std::endl;
            error_file_vector.push_back(xml_path);
            error_file_vector.push_back(img_vector[i]);
            continue;
        }
        
        if(dete_res->width != width)
        {
            std::cout << "error_size: " << xml_path << std::endl;
            std::cout << "error_size: " << img_vector[i] << std::endl;
            error_file_vector.push_back(xml_path);
            error_file_vector.push_back(img_vector[i]);
            continue;
        }

        // compare size_th
        for(int j=0; j<dete_res->size(); j++)
        {
            DeteObj dete_obj = dete_res->alarms[j];
            if((dete_obj.x2 < dete_obj.x1) || ((dete_obj.x2 - dete_obj.x1) < size_th) || (dete_obj.y2 < dete_obj.y1) || ((dete_obj.y2 - dete_obj.y1) < size_th))
            {
                std::cout << "error_size_th: " << xml_path << std::endl;
                std::cout << "error_size_th: " << img_vector[i] << std::endl;
                error_file_vector.push_back(xml_path);
                error_file_vector.push_back(img_vector[i]);
                continue;
            }
        }

        // delete
        if(remove_error_path)
        {
            for(int i=0; i<error_file_vector.size(); i++)
            {
                remove_file(error_file_vector[i]);
            }
        }

        delete dete_res;
    }
}


}