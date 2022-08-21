#ifndef _PARAMINFO_HPP_
#define _PARAMINFO_HPP_


#include <iostream>
#include <vector>
#include <map>


class ParamInfo
{
    public:
        // 参数名
        std::string command;
        // 英文说明
        std::string english_explain;
        // 中文解释
        std::string chinese_explain;
        // 语法
        std::string grammar;
        // 另外说明，注意点
        std::string describe;
        // 所属分区，比如 converse, filter, help, for_train, 
        std::string partition;
        // 实例
        std::vector<std::string> demo;

        // 构造函数
        ParamInfo(std::string command);
        // 是否为相似命令
        bool is_similar(std::string command); 
        // 

};


class UcdParamOpt
{
    // 参数字典
    std::map<std::string, ParamInfo> param_map;
    // 

    // 根据 command 获取参数实例
    ParamInfo get_param(std::string command);
    // 添加参数实例
    void add_param(ParamInfo param_info);
    // 是否有对应的 command 
    bool has_command(std::string command);
    // 寻找相似的command
    std::vector<std::string> find_similar_command(std::string );
    // 打印参数信息，一个或者多个参数
    void print_param_vector(std::vector<ParamInfo> param_info_vector);
    // 打印 command 对应的参数信息
    void print_info(std::string command);
    
};





#endif