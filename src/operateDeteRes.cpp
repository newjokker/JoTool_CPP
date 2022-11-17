

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
#include "include/md5.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <algorithm>
#include "ucDatasetUtil.hpp"
#include "include/tqdm.h"

#define ERROR_COLOR         "\x1b[35m"
#define WARNNING_COLOR      "\033[33m"
#define STOP_COLOR          "\033[0m"



namespace jotools
{


std::map<std::string, int> count_tags(std::string floder_path)
{
    std::map<std::string, int> count_res;
    std::vector<std::string> file_names = get_all_file_path_recursive(floder_path);
    std::set<std::string> suffixs;
    suffixs.insert(".xml");
    std::vector<std::string> file_name_xml = filter_by_suffix(file_names, suffixs);

    tqdm bar;
    int N = file_name_xml.size();

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
        bar.progress(i, N);
        delete dete_res;
    }
    bar.finish();
    // print res
    std::cout << "------------------------------------------" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(25) << "tags" << " :    " << "count" << std::left << std::setfill(' ') << std::setw(25) << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    int tags_numb = 0; 
    std::map<std::string, int>::iterator iter;
    iter = count_res.begin();
    while(iter != count_res.end()) 
    {
        std::cout << std::left << std::setfill(' ') << std::setw(25) << iter->first << " :    " << iter->second << std::left << std::setfill(' ') << std::setw(25) << std::endl;
        tags_numb += iter->second;
        iter++;
    }
    // extra info
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "number of xml has obj     :    " << file_name_xml.size() << std::endl;
    std::cout << "number of tag             :    " << tags_numb << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    return count_res;
}

void cut_small_img(std::string img_dir, std::string xml_dir, std::string save_dir, bool split_by_tag)
{
    std::vector<std::string> file_path_list = get_all_file_path(img_dir);
    std::vector<std::string> img_path_list;
    std::set<std::string> suffixs {".jpg", ".JPG", ".png", ".PNG"};
    img_path_list = filter_by_suffix(file_path_list, suffixs);

    for(int i=0; i<file_path_list.size(); i++)
    {
        std::cout << file_path_list[i] << std::endl;
    }

    std::cout << "here" << std::endl;
    std::cout << file_path_list.size() << std::endl;
    std::cout << img_path_list.size() << std::endl;

    for(int i=0; i<img_path_list.size();i++)
    {
        std::string each_xml_path = xml_dir + "/" + get_file_name(img_path_list[i]) + ".xml";
        
        std::cout << each_xml_path << std::endl;
        
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
    std::map<std::string, std::vector<std::string>> xml_info_map;
    std::vector<std::string> folder_path_list = get_all_folder_path(crop_dir);
    DeteObj obj;

    for(int i=0; i<folder_path_list.size(); i++)
    {
        std::vector<std::string> file_path_vector = get_all_file_path(folder_path_list[i]);

        std::set<std::string> suffixs {".jpg", ".png", ".JPG", ".PNG"};
        
        std::string folder_name = get_folder_name(folder_path_list[i]);

        std::vector<std::string> img_path_vector = filter_by_suffix(file_path_vector, suffixs);

        for(int j=0; j<img_path_vector.size(); j++)
        {
            std::string file_name = get_file_name(img_path_vector[j]);
            std::vector<std::string> loc_str_list = pystring::split(file_name, "-+-");
            std::string loc_str = loc_str_list[loc_str_list.size()-1];

            // 替换 loc_str 中的 tag 信息
            obj.load_from_name_str(loc_str);
            obj.tag = folder_name;
            loc_str = obj.get_name_str();

            std::string uc = file_name.substr(0, pystring::rfind(file_name, "-+-"));
            xml_info_map[uc].push_back(loc_str);
            // std::cout << uc << ", " << folder_name << ", " << loc_str << std::endl;
        }
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
            // 这边修改名字

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

void rename_all_files_by_md5(std::string folder_path)
{
    std::vector<std::string> file_path_vector = get_all_file_path_recursive(folder_path);

    std::string md5_str;
    std::string new_file_path;
    for(int i=0; i<file_path_vector.size(); i++)
    {
        md5_str = get_file_md5(file_path_vector[i]);
        new_file_path = get_file_folder(file_path_vector[i]) + "/" + md5_str + get_file_suffix(file_path_vector[i]);
        // std::cout << "md5 : " << md5_str << std::endl;
        // std::cout << "region path : " << file_path_vector[i] << std::endl;
        // std::cout << "rename path : " << new_file_path << std::endl;
        rename(file_path_vector[i].c_str(), new_file_path.c_str());
    }
}

void rename_xml_img_by_md5(std::string xml_folder, std::string img_folder)
{

    // 在使用这个代码的时候先要运行 xml_check 确保 xml 和 img 是一一对应的
    // 找到 xml 文件夹下面的所有 xml
    // 找到 img 文件夹中的所有 img

    if(!is_dir(xml_folder))
    {
        std::cout << "error, xml dir not exists : " << xml_folder << std::endl;
        throw "error, xml dir not exists : " + xml_folder;
    }

    if(!is_dir(img_folder))
    {
        std::cout << "error, img dir not exists : " << img_folder << std::endl;
        throw "error, xml dir not exists : " + xml_folder;
    }

    std::set<std::string> suffix {".jpg", ".png", ".JPG", ".PNG"};
    std::vector<std::string> img_path_vector = get_all_file_path(img_folder, suffix);

    std::string each_xml_path;
    std::string each_img_path;
    std::string md5_str;
    std::string new_xml_path, new_img_path;
    for(int i=0; i<img_path_vector.size(); i++)
    {
        each_img_path = img_path_vector[i];
        each_xml_path = xml_folder + "/" + get_file_name(each_img_path) + ".xml";
        if(! is_file(each_xml_path))
        {
            std::cout << "error, xml_path not exists : " << each_xml_path;
            throw "error, xml_path not exists : " + each_xml_path;
        }
        md5_str = get_file_md5(each_img_path);
        new_img_path = img_folder + "/" + md5_str + get_file_suffix(each_img_path);
        new_xml_path = xml_folder + "/" + md5_str + ".xml";
        rename(each_img_path.c_str(), new_img_path.c_str());
        rename(each_xml_path.c_str(), new_xml_path.c_str());
    }


}

float rect_iou(int x1, int y1, int h1, int w1, int x2, int y2, int h2, int w2)
{
    // 计算两个矩形的 iou
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

float dete_obj_iou(DeteObj a, DeteObj b)
{
    int x1 = a.x1;
    int y1 = a.y1;
    int h1 = a.y2 - a.y1;
    int w1 = a.x2 - a.x1;

    int x2 = b.x1;
    int y2 = b.y1;
    int h2 = b.y2 - b.y1;
    int w2 = b.x2 - b.x1;

    return rect_iou(x1, y1, h1, w1, x2, y2, h2, w2);
}

//


DeteAcc::DeteAcc()
{
    DeteAcc::iou = 0.5;
}

std::map<std::string, std::map<std::string, int> > DeteAcc::compare_customer_and_standard(DeteRes a, DeteRes b, std::string uc, UCDataset* compare_res_ucd)
{

    // sort by conf 
    a.sort_by_conf();
    b.sort_by_conf();

    DeteRes* compare_dete_res = new DeteRes();

    float iou_th = 0.5;
    std::map<int, bool> has_obj_map;
    std::map<int, bool> has_mis_map;                                    // 是否存在标签被标记为 mistake 
    std::map<std::string, std::map<std::string, int> > acc_res;
    acc_res["correct"]  = {};
    acc_res["miss"]     = {};
    acc_res["extra"]    = {};
    acc_res["mistake"]  = {};

    for(int i=0; i<b.alarms.size(); i++)
    {
        has_obj_map[i] = false;
        has_mis_map[i] = false;
    }

    for(int i=0; i<a.alarms.size(); i++)
    {
        DeteObj max_iou_obj;
        float max_iou = 0;
        int max_iou_index = -1;
        //
        for(int j=0; j<b.alarms.size(); j++)
        {
            float each_iou = dete_obj_iou(a.alarms[i], b.alarms[j]);
            if((each_iou > max_iou) && (has_obj_map[j] == false))
            {
                max_iou = each_iou;
                max_iou_obj = b.alarms[j];
                max_iou_index = j;
            }
        }

        if(max_iou > DeteAcc::iou)
        {
            if(a.alarms[i].tag == max_iou_obj.tag)
            {
                has_obj_map[max_iou_index] = true;
                DeteObj correct_dete_obj = a.alarms[i];
                correct_dete_obj.tag = "correct_" + a.alarms[i].tag;
                compare_dete_res->add_dete_obj(correct_dete_obj);
                if(acc_res["correct"].count(a.alarms[i].tag) == 0)
                {
                    acc_res["correct"][a.alarms[i].tag] = 1;
                }
                else
                {
                    acc_res["correct"][a.alarms[i].tag] += 1;
                }
            }
            else
            {
                DeteObj mistake_dete_obj = a.alarms[i];
                mistake_dete_obj.tag = "mistake_" + max_iou_obj.tag + ":" + a.alarms[i].tag;;
                has_mis_map[max_iou_index] = true;

                compare_dete_res->add_dete_obj(mistake_dete_obj);
                std::string mistake_str = a.alarms[i].tag;
                // std::string mistake_str = max_iou_obj.tag;
                if(acc_res["mistake"].count(mistake_str) == 0)
                {
                    acc_res["mistake"][mistake_str] = 1;
                }
                else
                {
                    acc_res["mistake"][mistake_str] += 1;
                }                
            }
        }
        else
        {
            DeteObj extra_dete_obj = a.alarms[i];
            extra_dete_obj.tag = "extra_" + a.alarms[i].tag;
            compare_dete_res->add_dete_obj(extra_dete_obj);
            if(acc_res["extra"].count(a.alarms[i].tag) == 0)
            {
                acc_res["extra"][a.alarms[i].tag] = 1;
            }
            else
            {
                acc_res["extra"][a.alarms[i].tag] += 1;
            }       
        }
    }

    // 检查漏检
    for(int i=0; i<b.alarms.size(); i++)
    {
        // 当一个标签既没有对应正确的标签也没有对应的错误标签，就属于漏掉的
        if((has_obj_map[i] == false) and (has_mis_map[i] == false))
        {
            DeteObj miss_dete_obj = b.alarms[i];
            miss_dete_obj.tag = "miss_" + b.alarms[i].tag;
            compare_dete_res->add_dete_obj(miss_dete_obj);
            //
            if(acc_res["miss"].count(b.alarms[i].tag) == 0)
            {
                acc_res["miss"][b.alarms[i].tag] = 1;
            }
            else
            {
                acc_res["miss"][b.alarms[i].tag] += 1;
            }   
        }
    }

    compare_res_ucd->add_dete_res_info(uc, *compare_dete_res);
    return acc_res;
}

std::map<std::string, std::map<std::string, int> > merge_compare_res(std::map<std::string, std::map<std::string, int> > b, std::map<std::string, std::map<std::string, int> > a)
{
    // 将两个检测结果进行合并
    auto iter_a = a.begin();
    while(iter_a != a.end())
    {
        std::map<std::string, int> each_res;
        each_res = iter_a->second;
        
        if(b.count(iter_a->first)==0)
        {
            b[iter_a->first] = {};
        }
        
        auto iter_a2 = each_res.begin();
        while(iter_a2 != each_res.end())
        {
            if(b[iter_a->first].count(iter_a2->first) == 0)
            {
                b[iter_a->first][iter_a2->first] = 0;
            }

            b[iter_a->first][iter_a2->first] += iter_a2->second;

            iter_a2++;
        }
        iter_a++;
    }
    return b;
}

std::map<std::string, std::vector<float> > DeteAcc::cal_acc_rec(UCDataset* ucd_a, UCDataset* ucd_b, std::string save_ucd_path, bool print)
{
    std::map<std::string, std::vector<float> > res;
    std::map<std::string, std::map<std::string, int> > compare_res;
    UCDataset* compare_res_ucd = new UCDataset();

    // 找到 a b uc 的合集
    std::set<std::string> uc_set;
    for(int i=0; i<ucd_a->uc_list.size(); i++)
    {
        uc_set.insert(ucd_a->uc_list[i]);
    }

    for(int i=0; i<ucd_b->uc_list.size(); i++)
    {
        uc_set.insert(ucd_b->uc_list[i]);
    }

    if(print)
    {
        tqdm bar;
        int N = uc_set.size();

        int i =0;
        auto iter_uc = uc_set.begin();
        while(iter_uc != uc_set.end())
        {
            std::string uc = iter_uc->data();
            DeteRes *each_b = new DeteRes();
            DeteRes *each_a = new DeteRes();
            ucd_b->get_dete_res_with_assign_uc(each_b, uc);
            ucd_a->get_dete_res_with_assign_uc(each_a, uc);
            std::map<std::string, std::map<std::string, int> > each_compare_res = DeteAcc::compare_customer_and_standard(*each_a, *each_b, uc, compare_res_ucd);
            compare_res = merge_compare_res(compare_res, each_compare_res);
            bar.progress(i, N);
            iter_uc ++;
            i++;
        }
        bar.finish();
    }
    else
    {
        int i =0;
        auto iter_uc = uc_set.begin();
        while(iter_uc != uc_set.end())
        {
            std::string uc = iter_uc->data();
            DeteRes *each_b = new DeteRes();
            DeteRes *each_a = new DeteRes();
            ucd_b->get_dete_res_with_assign_uc(each_b, uc);
            ucd_a->get_dete_res_with_assign_uc(each_a, uc);
            std::map<std::string, std::map<std::string, int> > each_compare_res = DeteAcc::compare_customer_and_standard(*each_a, *each_b, uc, compare_res_ucd);
            compare_res = merge_compare_res(compare_res, each_compare_res);            
            iter_uc ++;
            i++;
        }
    }

    // stastic
    std::set<std::string> all_tags      = ucd_a->get_tags();
    std::set<std::string> all_tags_b    = ucd_b->get_tags();
    all_tags.insert(all_tags_b.begin(), all_tags_b.end());
    
    if(print)
    {
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << "                                    STASTIC" << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << std::setw(20) << std::left << "TAG" << std::setw(10) << std::left << "CORRECT" << std::setw(10) << std::left << "MISS" << std::setw(10) << std::left << "EXTRA" 
                    << std::setw(10) << std::left << "MISTAKE" << std::setw(15) << std::left << "P(100%)" << std::setw(15) << std::left << "R(100%)" << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
    }

    int all_correct_num = 0;
    int all_miss_num    = 0;
    int all_extra_num   = 0;
    int all_mistake_num = 0;
    float all_p         = 0;
    float all_r         = 0;

    auto iter = all_tags.begin();
    while(iter != all_tags.end())
    {
        std::string tag = iter->data();
        int correct_num = 0;
        int miss_num    = 0;
        int extra_num   = 0;
        int mistake_num = 0;
        float each_p    = 0;
        float each_r    = 0;

        if(compare_res["correct"].count(tag) > 0)
        {
            correct_num = compare_res["correct"][tag];
        } 

        if(compare_res["extra"].count(tag) > 0)
        {
            extra_num   = compare_res["extra"][tag];
        } 

        if(compare_res["miss"].count(tag) > 0)
        {
            miss_num    = compare_res["miss"][tag];
        } 

        if(compare_res["mistake"].count(tag) > 0)
        {
            mistake_num = compare_res["mistake"][tag];
        } 

        if((correct_num + mistake_num + extra_num) > 0)
        {
            each_p = 100 * (correct_num * 1.0) / (correct_num + mistake_num + extra_num);
        }

        if((correct_num + miss_num) > 0)
        {
            each_r = 100 * (correct_num * 1.0) / (correct_num + miss_num);
        } 

        all_correct_num += correct_num;
        all_extra_num   += extra_num;
        all_miss_num    += miss_num;
        all_mistake_num += mistake_num;
        

        res[tag].push_back(each_p);
        res[tag].push_back(each_r);

        if(print)
        {
            std::cout << std::setw(20) << std::left << tag << std::setw(10) << std::left << correct_num << std::setw(10) << std::left << miss_num << std::setw(10) << std::left << extra_num 
                    << std::setw(10) << std::left << mistake_num << std::setw(15) << std::left << each_p << std::setw(15) << std::left << each_r << std::endl;
        }
        iter++;
    } 


    if((all_correct_num + all_mistake_num + all_extra_num) > 0)
    {
        all_p = 100 * (all_correct_num * 1.0) / (all_correct_num + all_mistake_num + all_extra_num);
    }

    if((all_correct_num + all_miss_num) > 0)
    {
        all_r = 100 * (all_correct_num * 1.0) / (all_correct_num + all_miss_num);
    } 

    if(print)
    {
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << std::setw(20) << std::left << "TOTAL" << std::setw(10) << std::left << all_correct_num << std::setw(10) << std::left << all_miss_num << std::setw(10) << std::left << all_extra_num 
                << std::setw(10) << std::left << all_miss_num << std::setw(15) << std::left << all_p << std::setw(15) << std::left << all_r << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << "IOU : " << DeteAcc::iou << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << "" << std::endl;
        std::cout << WARNNING_COLOR << "* 一横排全部统计为 0 是没有检测到也没有遗漏，是被误检了其他类型了，不是误检到当前类型，所以误检不统计" << STOP_COLOR << std::endl;
        std::cout << WARNNING_COLOR << "* 这个函数还需要再测测，看看否有遗漏的问题" << STOP_COLOR << std::endl;
    }

    compare_res_ucd->size_info = ucd_b->size_info;
    if(save_ucd_path.size() > 0)
    {
        compare_res_ucd->save_to_ucd(save_ucd_path);
    }
    delete compare_res_ucd;
    return res;
}

static float vector_means(std::vector<float> res)
{
    if(res.size() == 0)
    {
        return 0;
    }

    float res_sum = 0;
    for(int i=0; i<res.size(); i++)
    {
        res_sum += res[i];
    }
    return res_sum / res.size();
}

void DeteAcc::cal_map(UCDataset* ucd_a, UCDataset* ucd_b)
{
    std::cout << "---------------------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::setw(20) << std::left << "TAG         (P/R)";
    std::vector<std::string> ap_list;
    
    // 各个类别的 ap 的均值就是 map

    std::map< std::string, std::vector<float> > tag_ap_dict; 
    std::map< std::string, std::vector<float> > tag_ar_dict; 

    for(int i=5; i<100; i+=10)
    {
        ucd_a->filter_by_conf(i * 0.01, false);
        std::map<std::string, std::vector<float> > each_res = DeteAcc::cal_acc_rec(ucd_a, ucd_b, "", false);
        auto iter = each_res.begin();
        while(iter != each_res.end())
        {
            std::string tag = iter->first;
            tag_ap_dict[tag].push_back(iter->second[0]);
            tag_ar_dict[tag].push_back(iter->second[1]);
            iter++;
        }
        std::cout << std::setw(8) << std::left << i;
    }
    std::cout << std::setw(8) << std::left << "AP (100%)" << std::endl;
    std::cout << "---------------------------------------------------------------------------------------------------------------" << std::endl;

    auto iter_ap = tag_ap_dict.begin();
    while(iter_ap != tag_ap_dict.end())
    {
        // float each_ap = vector_means(iter_ap->second);
        // percion
        std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(2);
        std::cout << std::setw(20) << std::left << iter_ap->first;
        for(int j=0; j<iter_ap->second.size(); j++)
        {
            std::cout << std::setw(8) << std::left << iter_ap->second[j];
        }
        std::cout << std::endl;
        // recall
        std::cout << std::setw(20) << std::left << " ";
        for(int x=0; x<tag_ar_dict[iter_ap->first].size(); x++)
        {
            std::cout << std::setw(8) << std::left << tag_ar_dict[iter_ap->first][x];
        }
        std::cout << "wait_for_cal" << std::endl;
        std::cout << "                    -------------------------------------------------------------------------------------------" << std::endl;
        iter_ap++;
    }

    std::cout << "---------------------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::setw(10) << std::left << "AVERAGE" << std::setw(10) << std::left << "P(100%)" << std::setw(10) << std::left << "R(100%)" 
              << std::setw(10) << std::left << "IOU : " + std::to_string(DeteAcc::iou) << std::endl;
    std::cout << "---------------------------------------------------------------------------------------------------------------" << std::endl;

    std::cout << WARNNING_COLOR << "wait for cal" << STOP_COLOR << std::endl;
}


}