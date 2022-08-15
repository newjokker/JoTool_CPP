

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
        double add_time;
        double update_time;
        std::string describe;
        std::string json_path;
        //
        UCDataset(std::string json_path="");
        // 解析 json 数据
        void parse_json_info();
        // 保存为 json
        void save_to_json(std::string save_path);
        // 打印 json 数据
        void print_json_info();
        // uc list 去重
        void unique();
};


class UCDatasetUtil
{
    public:
        //
        UCDatasetUtil(std::string host, int port);
        std::string json_path;
        std::string host;
        std::string root_url;
        int port;
        // 下载 ucd 中对应的数据
        void save_img_xml_json(std::string save_dir, bool need_img=true, bool need_xml=true, bool need_json=false, int need_count=-1);
        // 下载库中的 ucd
        void save_ucd(std::string ucd_name, std::string save_dir);
        // 查看库中的 ucd
        void check_ucd();
        // 删除库中的 ucd
        void delete_ucd(std::string ucd_name);
        // 上传 ucd
        void upload_ucd(std::string ucd_path, std::string ucd_name="");
        // 将 img_dir 中的文件符合 UC 规范的组织为一个 ucd
        void get_ucd_from_img_dir(std::string img_dir, std::string ucd_path);
        // 将多个 ucd 进行合并
        void merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector);
        // 查看两个 ucd 之间的差异
        void ucd_diff(std::string ucd_path_1, std::string ucd_path_2);
        // 去除在 ucd1 中 ucd2 中也存在的 uc
        void ucd_minus(std::string save_path, std::string ucd_path_1, std::string ucd_path_2);
    private:
        // 下载云上的数据
        void load_file(std::string url, std::string save_path, int index=-1);
};





#endif