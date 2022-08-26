

#ifndef _UCDATASETUTIL_HPP_
#define _UCDATASETUTIL_HPP_

#include <iostream>
#include <vector>
#include "include/lablelmeObj.hpp"

class UCDataset
{
    public:
        //
        std::vector<std::string> uc_list;
        std::string dataset_name;
        std::string model_name;
        std::string model_version;
        std::vector<std::string> label_used;
        // {uc: [uc, x1, y1, x2, y2, conf, tag]}
        std::map<std::string, std::vector<std::vector<std::string> > > xml_info; 
        // labelme 中的对象 {uc: [PointObj, CricleObj, retangleObj ...]}
        std::map<std::string, std::vector<LabelmeObj*> > object_info;
        double add_time;
        double update_time;
        std::string describe;
        //
        UCDataset(std::string ucd_path="");
        
        // ~UCDataset();
        
        // 解析 ucd 数据
        void parse_ucd(bool parse_xml_info=false);
        
        // 打印 json 数据
        void print_ucd_info();
        
        // 打印指定 uc 对应的信息
        void print_assign_uc_info(std::string uc);
        
        // uc list 去重
        void unique();
        
        // 指定 uc 中是否存在某个对象
        bool has_obj(std::string uc, LabelmeObj *obj);
        
        // 对 uc 进行切片
        std::vector<std::string> uc_slice(int start, int end);
        
        // 统计标签个数
        std::map<std::string, int> count_tags();
        
        // 修改属性
        void change_attar(std::string attr_name, std::string attr_value);

        // 增量解析 xml 数据到 ucd 中
        void add_voc_xml_info(std::string uc, std::string voc_xml_path);
        
        // 增量解析 labelme 的 json 数据
        void add_labelme_json_info(std::string uc, std::string labelme_json_path);
        
        // 增量解析 saturndatabase 的 json 数据
        void add_saturndatabase_json_info(std::string uc, std::string labelme_json_path);

        // 保存为 ucd（斜框矩形如何进行保存）
        void save_to_ucd(std::string save_path);
        
        // 保存为 voc 的xml 形式（只保存正框，或者其他类型的外接矩形代表的正框）
        void save_to_voc_xml(std::string save_path);
        
        // 保存为 labelme 的 json 形式（所有对象都保存，斜框矩阵是都进行保存）
        void save_to_labelme_json(std::string save_path);

    private:
        std::string json_path;
};


class UCDatasetUtil
{
    public:
        // 缓存文件夹，多个人多个项目使用的时候可以直接缓存图片
        std::string cache_dir;
        std::string cache_img_dir;
        std::string cache_xml_dir;
        
        // ucd json_path
        std::string json_path;

        // 下载服务的主机地址
        std::string host;

        // 下载服务的下载路径
        std::string root_url;
        
        // 下载服务器的端口号
        int port;
        
        // 构造函数 
        UCDatasetUtil(std::string host, int port, std::string cache_dir="");
        
        // 从服务器下载对应的数据
        void save_img_xml_json(std::string save_dir, bool need_img=true, bool need_xml=true, int need_count=-1);
        
        // 下载图像 | xml | ucd
        void load_img(std::string save_dir, std::vector<std::string> uc_list);
        void load_xml(std::string save_dir, std::vector<std::string> uc_list);
        void load_ucd(std::string ucd_name, std::string save_dir);
        
        // 从 json 中解析 xml 并保存
        void save_to_xml(std::string save_path, std::vector<std::string>);
        
        // 查看库中的 ucd
        void search_ucd();
        
        // 是不是 ucd path （1）是不是合法文件 （2）是否为 .json 结尾的文件
        bool is_ucd_path(std::string ucd_path);
        
        // 删除库中的 ucd
        void delete_ucd(std::string ucd_name);
        
        // 上传 ucd
        void upload_ucd(std::string ucd_path, std::string ucd_name="");
        
        // 将 img_dir 中的文件符合 UC 规范的组织为一个 ucd
        void get_ucd_from_img_dir(std::string img_dir, std::string ucd_path);
        
        // 将 xml 信息 保存在 ucd 中
        void get_ucd_from_xml_dir(std::string xml_dir, std::string ucd_path);
        
        // 将 labelme json 信息保存到 ucd 中
        void get_ucd_from_json_dir(std::string json_dir, std::string ucd_path);
        
        // 将多个 ucd 进行合并
        void merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector);
        
        // 查看两个 ucd 之间的差异
        void ucd_diff(std::string ucd_path_1, std::string ucd_path_2);
        
        // 去除在 ucd1 中 ucd2 中也存在的 uc
        void ucd_minus(std::string save_path, std::string ucd_path_1, std::string ucd_path_2);
        
        // 统计标签的个数
        void count_ucd_tags(std::string ucd_path);
        
        // 清空缓存
        void cache_clear();
        void cache_clear(std::string ucd_path);
        
        // 打印文字
        void print_words(std::string name, int width=50, int height=50);
        
        // 裁切小图
        void cut_small_img(std::string img_dir, std::string xml_dir, std::string save_dir, bool is_split);
        
    private:
        
        // 下载云上的数据
        void load_file(std::string url, std::string save_path, int index=-1);
};





#endif