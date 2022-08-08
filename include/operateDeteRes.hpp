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

    void get_xml_from_crop_img(std::string crop_dir,  std::string save_xml_dir);

    // 去除图片的元数据
    void remove_image_meta_info(std::string img_path);

    // 查看文件分布
    void count_files(std::string folder_path, bool recursive=true);

}


#endif