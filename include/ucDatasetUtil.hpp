

#ifndef _UCDATASETUTIL_HPP_
#define _UCDATASETUTIL_HPP_

#include <iostream>
#include <vector>


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
        double add_time;
        double update_time;
        std::string describe;
        //
        UCDataset(std::string json_path);
        // 解析 json 数据
        void parse_json_info(bool parse_xml_info=false);
        // 保存为 json
        void save_to_json(std::string save_path);
        // 打印 json 数据
        void print_json_info();
        // uc list 去重
        void unique();
        // 统计标签个数
        std::map<std::string, int> count_tags();
    private:
        std::string json_path;
};


class UCDatasetUtil
{
    public:
        // 缓存文件夹，多个人多个项目使用的时候可以直接缓存图片
        std::string cache_dir;
        std::string cache_img_dir;
        std::string json_path;
        std::string host;
        std::string root_url;
        int port;
        // 
        UCDatasetUtil(std::string host, int port, std::string cache_dir="");
        // 下载 ucd 中对应的数据
        void save_img_xml_json(std::string save_dir, bool need_img=true, bool need_xml=true, int need_count=-1);
        // 下载库中的 ucd
        void save_ucd(std::string ucd_name, std::string save_dir);
        // 从 json 中解析 xml
        void save_xml(std::string save_path, int get_count=-1);
        // 查看库中的 ucd
        void check_ucd();
        // 是不是 ucd path （1）是不是合法文件 （2）是否为 .json 结尾的文件
        bool is_ucd_path(std::string ucd_path);
        // 删除库中的 ucd
        void delete_ucd(std::string ucd_name);
        // 上传 ucd
        void upload_ucd(std::string ucd_path, std::string ucd_name="");
        // 将 img_dir 中的文件符合 UC 规范的组织为一个 ucd
        void get_ucd_from_img_dir(std::string img_dir, std::string ucd_path);
        // 将 img 和 xml 信息 保存在 ucd 中
        void get_ucd_from_img_xml_dir(std::string img_dir, std::string xml_dir, std::string ucd_path);
        // 将多个 ucd 进行合并
        void merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector);
        // 查看两个 ucd 之间的差异
        void ucd_diff(std::string ucd_path_1, std::string ucd_path_2);
        // 去除在 ucd1 中 ucd2 中也存在的 uc
        void ucd_minus(std::string save_path, std::string ucd_path_1, std::string ucd_path_2);
        // 统计标签的个数
        void count_ucd_tags(std::string ucd_path);
    private:
        // 下载云上的数据
        void load_file(std::string url, std::string save_path, int index=-1);
};





#endif