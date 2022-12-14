

#ifndef _UCDATASETUTIL_HPP_
#define _UCDATASETUTIL_HPP_

#include <iostream>
#include <vector>
#include "include/lablelmeObj.hpp"
#include "include/deteRes.hpp"
using namespace jotools;


class UCDataset
{
    public:
        // uc 列表
        std::vector<std::string> uc_list;
        
        // 数据集名字
        std::string dataset_name;
        
        // 模型名字
        std::string model_name;
        
        // 模型版本
        std::string model_version;
        
        // 使用的标签
        std::vector<std::string> label_used;

        // labelme 中的对象 {uc: [PointObj, CricleObj, retangleObj ...]}
        std::map<std::string, std::vector<LabelmeObj*> > object_info;

        // size_info {uc: [width, height]}
        std::map<std::string, std::vector<int> > size_info;

        // ucd 新建的时间
        double add_time;
        
        // ucd 更新的时间
        double update_time;

        // ucd 的描述
        std::string describe;
        
        // 构造函数
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
        
        void add_obj(std::string uc, LabelmeObj *obj);

        // 删除 obj
        void delete_obj(std::string uc, LabelmeObj *obj); 

        // 对 uc 进行切片
        std::vector<std::string> uc_slice(int start, int end);
        
        // 统计标签个数
        std::map<std::string, std::map<std::string, int> > count_tags();
        
        // 修改属性
        void change_attar(std::string attr_name, std::string attr_value);

        // 增量解析 xml 数据到 ucd 中
        void add_voc_xml_info(std::string uc, std::string voc_xml_path);

        // 增量 添加 deteRes 数据到 ucd 中
        void add_dete_res_info(std::string uc, DeteRes dete_res);
        
        // 增量解析 labelme 的 json 数据
        void add_labelme_json_info(std::string uc, std::string labelme_json_path);

        // 增量解析 saturndatabase 的 json 数据
        void add_saturndatabase_json_info(std::string uc, std::string labelme_json_path);

        // 增量解析 ucd_info
        void add_ucd_info(std::string ucd_path);

        // 保存为 ucd（斜框矩形如何进行保存）
        void save_to_ucd(std::string save_path);
        
        // 保存一个 xml 文件
        void save_to_voc_xml_with_assign_uc(std::string save_path, std::string img_path, std::string assign_uc);

        // 保存一个 json 文件
        void save_to_labelme_json_with_assign_uc(std::string save_path, std::string img_path, std::string assign_uc);

        // save_to_yolo_train_data
        void save_to_yolo_train_txt_with_assign_uc(std::string save_path, std::string img_path, std::string assign_uc, std::vector<std::string> label_list);

        // 保存为一个 dete_res
        void get_dete_res_with_assign_uc(jotools::DeteRes* dete_res, std::string assign_uc);

        // 根据阈值进行过滤
        void filter_by_conf(float conf_th);

        // 根据标签进行过滤
        void filter_by_tags(std::vector<std::string> tags);

        // 根据面积进行过滤
        void filter_by_area(float area_th);

        // 对 ucd 做 nms
        void filter_by_nms(float nms_th, bool ignore_tag);

        // crop_dete_res
        void crop_dete_res_with_assign_uc(std::string uc, std::string img_path, std::string save_dir);

        // 获得子序列
        void get_sub_ucd(int sub_count, bool is_random, std::string save_path);

        // 去除没有 object_info 的uc
        void update_uc_list_by_object_info(std::string save_path);

        // 随机将 ucd 分割为一定比例的几个部分
        void split(std::string ucd_part_a, std::string ucd_part_b, float ratio);

        // 从指定 ucd 中提取需要的内容
        void absorb(std::string meat_ucd, std::string save_path, std::string need_attr);

        // 将数据集随机平分为几部分
        void devide(std::string save_path, int devide_count);

        // exec command 中的子任务
        void command_ADD(std::vector<std::string> tokens);
        void command_DROP(std::vector<std::string> tokens);
        void command_SET(std::vector<std::string> tokens);

        // 执行 ucd command 脚本, 保存为新的 ucd 
        void exec(std::string command_path);

        // 过滤标签
        void filter_by_tags(std::set<std::string> tags);

        // 抛弃标签
        void drop_tags(std::set<std::string> tags);

        // 更新标签
        void update_tags(std::map< std::string, std::string > tag_map);

        // 舍弃没有 obj 的 uc
        void drop_empty_uc();

        // 拿到不重复的标签
        std::set<std::string> get_tags();

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
        std::string cache_crop_dir;
        std::string color_file;

        // 颜色映射字典
        std::map< std::string, Color > color_map;
        
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
        void load_img_with_assign_uc(std::string save_dir, std::string assign_uc);
        void load_xml(std::string save_dir, std::vector<std::string> uc_list);
        void load_ucd(std::string ucd_name, std::string save_dir);
        void load_ucd_app(std::string version, std::string save_path);
        
        // 从 ucd 中解析指定的 uc 的 labelme json 文件
        void parse_labelme_json(std::string img_dir, std::string save_dir, std::string ucd_path);

        // 从 ucd 中解析出指定 uc 的 voc_xml 文件 （img_dir 用于读取图片的长宽，要是不存在的话那么就设置为 -1）
        void parse_voc_xml(std::string img_dir, std::string save_dir, std::string ucd_path);

        // to yolo train_data
        void parse_yolo_train_data(std::string img_dir, std::string save_dir, std::string ucd_path, std::vector<std::string> label_list = {});
        
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
        
        // 从文件中获取 ucd，只解析文件名，不解析文件内容
        void get_ucd_from_file_dir(std::string file_dir, std::string ucd_path);

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
        void cut_small_img(std::string ucd_path, std::string save_dir, bool is_split);

        // uc_check
        void uc_check(std::vector<std::string> file_vector);

        // uc_analysis
        void uc_analysis(std::string ucd_path);

        // conf_analysis
        void conf_analysis(std::string ucd_path, int seg_count);

        // area_analysis
        void area_analysis(std::string ucd_path, int seg_count);

        // 缓存文件清洗
        void cache_clean(std::string clean_folder);

        // 将文件赋予假的uc，FakeUC
        void set_fack_uc(std::string folder_path);

        // 对检测结果进行画图
        void draw_res(std::string ucd_path, std::string save_dir, std::vector<std::string> uc_list);

        // 随机给不同的标签分配不同的颜色
        void get_random_color_map(std::string ucd_path);


    private:
        
        // 下载云上的数据
        void load_file(std::string url, std::string save_path, int index=-1);
};





#endif