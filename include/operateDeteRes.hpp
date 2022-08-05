#ifndef _OPERATEDETERES_HPP_
#define _OPERATEDETERES_HPP_


#include <iostream>
#include <vector>
#include <map>
#include "./deteRes.hpp"
#include "./fileOperateUtil.hpp"

// hpp 和 cpp 要使用同样的命名空间
namespace jotools
{
    std::map<std::string, int> count_tags(std::string floder_path);

    void cut_small_img(std::string img_dir, std::string xml_dir, std::string save_dir, bool split_by_tag);

    void get_xml_from_crop_img(std::string crop_dir, std::string region_img_dir, std::string save_xml_dir);

    void test();

}


#endif