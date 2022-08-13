

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
        //
        void parse_json_info();
        void save_to_json(std::string save_path);
        void print_json_info();
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
        //
        void save_img_xml_json(std::string save_dir, bool need_img=true, bool need_xml=true, bool need_json=false, int need_count=-1);
        void save_ucd(std::string ucd_name, std::string save_dir);
        //
        void check_ucd();
        void delete_ucd(std::string ucd_name);
        void upload_ucd(std::string ucd_path, std::string ucd_name="");
        // 将 img_dir 中的文件符合 UC 规范的组织为一个 ucd
        void get_ucd_from_img_dir(std::string img_dir, std::string ucd_path);
        // 将多个 ucd 进行合并
        void merge_ucds(std::string save_path, std::vector<std::string> ucd_path_vector);

    private:
        void load_file(std::string url, std::string save_path, int index=-1);
};





#endif