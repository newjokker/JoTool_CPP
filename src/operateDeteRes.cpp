

#include <iostream>
#include <vector>
#include <map>
#include "../include/deteRes.hpp"
#include "../include/fileOperateUtil.hpp"


namespace jotools
{

std::map<std::string, int> count_tags(std::string floder_path)
{
    // todo 增加标签总数统计
    // todo 增加解析的 xml 的个数
    std::map<std::string, int> count_res;
    std::vector<std::string> file_names;
    GetFileNames(floder_path, file_names);
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
    std::vector<std::string> file_path_list;
    GetFileNames(img_dir, file_path_list);

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

}