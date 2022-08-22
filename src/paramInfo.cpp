


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "include/paramInfo.hpp"

// using namespace std;



int min_edit_cost(std::string s1, std::string s2, int ic, int dc, int rc) 
{
    // s1, s2 两个字符串计算最小编辑代价，ic insert, dc delete, rc replace 
    int m=s1.length(),n=s2.length();
    std::vector<std::vector<int> > v(m+1, std::vector<int>(n+1,0));
    for(int i=0;i<m+1;i++) 
    {
        for(int j=0;j<n+1;j++) 
        {
            if(i==0 && j!=0) v[i][j]=j*ic;
            else if(i!=0 && j==0) v[i][j]=i*dc;
            else if(i==0 && j==0) v[i][j]=0;
            else if(s1[i-1]==s2[j-1]) v[i][j]=v[i-1][j-1];
            else v[i][j] = std::min(std::min(v[i-1][j]+dc,v[i][j-1]+ic),v[i-1][j-1]+rc);
        }
    }
    return v[m][n];
}

ParamInfo::ParamInfo(std::string command)
{
    ParamInfo::command = command;

    // std::cout << "command is : " << command << std::endl;   

    // if(command == "")
    // {
    //     std::cout << "command is empty" << std::endl;
    //     throw "command is empty";
    // }
    // else
    // {
    //     ParamInfo::command = command;
    // }
}

bool ParamInfo::is_similar(std::string other_command)
{
    if(ParamInfo::command == other_command)
    {
        return true;
    }

    // 字符串的相似程度就是计算字符串的最小编辑代价
    int mec = min_edit_cost(ParamInfo::command, other_command, 1, 1, 1);
    float diff_index = (float)mec / (float)ParamInfo::command.size();

    // std::cout << "diff index : " << diff_index << std::endl;

    if(diff_index < 0.5)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ParamInfo::print_info()
{
    std::cout << ParamInfo::command << std::endl;
    std::cout << "   " << ParamInfo::grammar << std::endl;
    std::cout << "   " << ParamInfo::english_explain << std::endl;
    std::cout << "   " << ParamInfo::chinese_explain << std::endl;
}


//

ParamInfo UcdParamOpt::get_param(std::string command)
{
    if(UcdParamOpt::param_map.count(command) == 0)
    {
        std::cout << "command not register : " << command << std::endl;
        throw "command not register";
    }
    else
    {
        return UcdParamOpt::param_map[command];
    }
}

bool UcdParamOpt::has_command(std::string command)
{
    if(UcdParamOpt::param_map.count(command) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool UcdParamOpt::has_simliar_command(std::string command)
{
    std::vector<std::string> similar_command_vector = UcdParamOpt::find_similar_command(command);
    if(similar_command_vector.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UcdParamOpt::add_param(ParamInfo *param_info)
{
    if(UcdParamOpt::has_command(param_info->command))
    {
        std::cout << "command exists : " << param_info->command << std::endl;
        throw "command exists";
    }
    else
    {
        UcdParamOpt::param_map[param_info->command] = *param_info;
    }
}

std::vector<std::string> UcdParamOpt::find_similar_command(std::string command)
{
    std::vector<std::string> command_vector;
    auto iter = UcdParamOpt::param_map.begin();
    while (iter != UcdParamOpt::param_map.end())
    {
        if(iter->second.is_similar(command))
        {
            command_vector.push_back(iter->second.command);
        }
        iter++;
    }
    return command_vector;
}

void UcdParamOpt::print_similar_command_info(std::string command)
{
    std::cout << "can not find command : " << command << ", find similar command :" << std::endl; 
    std::cout << std::endl;
    std::vector<std::string> similar_command_vector = UcdParamOpt::find_similar_command(command);
    if(similar_command_vector.size() > 0)
    {
        std::cout << "------------------------" << std::endl;

        for(int i=0; i<similar_command_vector.size(); i++)
        {
            ParamInfo param_info = UcdParamOpt::get_param(similar_command_vector[i]);
            param_info.print_info();
            std::cout << " " << std::endl;
        }
        std::cout << "------------------------" << std::endl;
    }
}

void UcdParamOpt::print_param_vector(std::vector<ParamInfo> param_info_vector)
{
    //
    std::cout << "------------------------" << std::endl;
    for(int i=0; i<param_info_vector.size(); i++)
    {
        // 
        param_info_vector[i].print_info();
    }
    std::cout << "------------------------" << std::endl;
}

void UcdParamOpt::print_command_info(std::string command)
{
    std::cout << "------------------------" << std::endl;
    ParamInfo param_info = UcdParamOpt::get_param(command);
    param_info.print_info();
    std::cout << "------------------------" << std::endl;
}

void UcdParamOpt::print_command_info(std::vector<std::string> command_vector)
{
    //
    std::cout << "------------------------" << std::endl;
    for(int i=0; i<command_vector.size(); i++)
    {
        ParamInfo param_info = UcdParamOpt::get_param(command_vector[i]);
        param_info.print_info();
    }
    std::cout << "------------------------" << std::endl;
}

void UcdParamOpt::print_all_fun()
{
    // 计算模块属于的组，按照分组打印所有的帮助信息
    std::map<std::string, std::set<std::string> > gropu_map; 
    auto iter = UcdParamOpt::param_map.begin();
    while(iter != UcdParamOpt::param_map.end())
    {
        gropu_map[iter->second.group].insert(iter->first);
        iter ++ ;
    }
    // 按照分组打印所有命令
    std::cout << "------------------------" << std::endl;
    auto iter_group = gropu_map.begin();
    while(iter_group != gropu_map.end())
    {
        std::cout << iter_group->first << std::endl;
        auto iter_command = iter_group->second.begin();
        while(iter_command != iter_group->second.end())
        {
            ParamInfo param_info = UcdParamOpt::get_param(iter_command->data());
            std::cout << std::setw(20) << std::right << iter_command->data() << "," ;
            std::cout << std::setw(4) << std::left << " " << param_info.grammar << std::endl;
            iter_command++;
        }
        iter_group++;
    }
    std::cout << "------------------------" << std::endl;
}

void UcdParamOpt::load_param_info()
{

    // check
    ParamInfo * param_check = new ParamInfo("check");
    param_check->group = "common";
    param_check->grammar = "ucd check";
    param_check->english_explain = "get all ucd official|customer from server";
    param_check->chinese_explain = "查看服务器中所有 ucd 信息，包括官方 ucd 和 自定义 ucd";
    UcdParamOpt::add_param(param_check);
    
    // save
    ParamInfo * param_save = new ParamInfo("save");
    param_save->group = "common";
    param_save->grammar = "ucd save json_path save_dir save_mode(image,xml,json) {need_count}";
    param_save->english_explain = "load img|xml from server";
    param_save->chinese_explain = "从服务器下载 图片|标注";   
    UcdParamOpt::add_param(param_save);
    
    // parse
    ParamInfo * param_parse = new ParamInfo("parse");
    param_parse->group = "common";
    param_parse->grammar = "ucd parse json_path save_dir save_mode(image,xml_from_json) {need_count}";
    param_parse->english_explain = "parse xml from json";
    param_parse->chinese_explain = "下载 图片 可以选择从 json 解析出 xml";   
    UcdParamOpt::add_param(param_parse);
    
    // show
    ParamInfo * param_show = new ParamInfo("show");
    param_show->group = "common";
    param_show->grammar = "ucd show {uc}";
    param_show->english_explain = "show for all url supported";
    param_show->chinese_explain = "查看所有下载路径";   
    UcdParamOpt::add_param(param_show);
    
    // delete
    ParamInfo * param_delete = new ParamInfo("delete");
    param_delete->group = "ucd_opt";
    param_delete->grammar = "ucd show {uc}";
    param_delete->english_explain = "ucd delete ucd_name";
    param_delete->chinese_explain = "删除在线数据集,无法删除官方数据集";   
    UcdParamOpt::add_param(param_delete);
    
    // load
    ParamInfo * param_load = new ParamInfo("load");
    param_load->group = "ucd_opt";
    param_load->grammar = "ucd load ucd_name save_path|save_dir";
    param_load->english_explain = "load ucd from server";
    param_load->chinese_explain = "下载在线数据集";   
    UcdParamOpt::add_param(param_load);
    
    // upload
    ParamInfo * param_upload = new ParamInfo("upload");
    param_upload->group = "ucd_opt";
    param_upload->grammar = "ucd upload ucd_path {ucd_name}";
    param_upload->english_explain = "upload ucd to server";
    param_upload->chinese_explain = "上传数据集到网络";   
    UcdParamOpt::add_param(param_upload);
    
    // from_img
    ParamInfo * param_from_img = new ParamInfo("from_img");
    param_from_img->group = "ucd_opt";
    param_from_img->grammar = "ucd from img_dir ucd_save_path";
    param_from_img->english_explain = "get ucd from loacal data(img)";
    param_from_img->chinese_explain = "本地文件生成数据集";   
    UcdParamOpt::add_param(param_from_img);
    
    // from_img_xml
    ParamInfo * param_from_img_xml = new ParamInfo("from_img_xml");
    param_from_img_xml->group = "ucd_opt";
    param_from_img_xml->grammar = "ucd from img_dir ucd_save_path";
    param_from_img_xml->english_explain = "get ucd from loacal data(img, xml)";
    param_from_img_xml->chinese_explain = "本地文件生成 ucd, ucd中包含 dete_obj 信息";   
    UcdParamOpt::add_param(param_from_img_xml);
    
    // info
    ParamInfo * param_info = new ParamInfo("info");
    param_info->group = "ucd_opt";
    param_info->grammar = "ucd info ucd_path";
    param_info->english_explain = "show ucd info";
    param_info->chinese_explain = "查看数据集信息";   
    UcdParamOpt::add_param(param_info);
    
    // meta
    ParamInfo * param_meta = new ParamInfo("meta");
    param_meta->group = "ucd_opt";
    param_meta->grammar = "ucd meta";
    param_meta->english_explain = "show config info";
    param_meta->chinese_explain = "查看配置信息";   
    UcdParamOpt::add_param(param_meta);
    
    // set
    ParamInfo * param_set = new ParamInfo("set");
    param_set->group = "ucd_opt";
    param_set->grammar = "ucd set key value";
    param_set->english_explain = "update config info";
    param_set->chinese_explain = "设置配置信息";   
    UcdParamOpt::add_param(param_set);
    
    // param_merge
    ParamInfo * param_merge = new ParamInfo("merge");
    param_merge->group = "ucd_opt";
    param_merge->grammar = "ucd merge save_path ucd_path1 ucd_path2 ...";
    param_merge->english_explain = "merge ucd info";
    param_merge->chinese_explain = "合并数据集";   
    UcdParamOpt::add_param(param_merge);
    
    // minus
    ParamInfo * param_minus = new ParamInfo("minus");
    param_minus->group = "ucd_opt";
    param_minus->grammar = "ucd minus save_path ucd_path1 ucd_path2";
    param_minus->english_explain = "do minus operation between two ucd";
    param_minus->chinese_explain = "减操作数据集";   
    UcdParamOpt::add_param(param_minus);
    
    // diff
    ParamInfo * param_diff = new ParamInfo("diff");
    param_diff->group = "ucd_opt";
    param_diff->grammar = "ucd diff ucd_path1 ucd_path2";
    param_diff->english_explain = "compare two ucd";
    param_diff->chinese_explain = "比较数据集";   
    UcdParamOpt::add_param(param_diff);
    
    // rename_img
    ParamInfo * param_rename_img = new ParamInfo("rename_img");
    param_rename_img->group = "ucd_opt";
    param_rename_img->grammar = "ucd rename_img img_dir";
    param_rename_img->english_explain = "rename img by uc";
    param_rename_img->chinese_explain = "重命名数据集";   
    UcdParamOpt::add_param(param_rename_img);
    
    // rename_img_xml
    ParamInfo * param_rename_img_xml = new ParamInfo("rename_img_xml");
    param_rename_img_xml->group = "ucd_opt";
    param_rename_img_xml->grammar = "ucd rename_img_xml img_dir xml_dir";
    param_rename_img_xml->english_explain = "rename img xml by uc";
    param_rename_img_xml->chinese_explain = "重命名数据集";   
    UcdParamOpt::add_param(param_rename_img_xml);
    
    // count_tags
    ParamInfo * param_count_tags = new ParamInfo("count_tags");
    param_count_tags->group = "ucd_opt";
    param_count_tags->grammar = "ucd count_tags xml_dir";
    param_count_tags->english_explain = "count tags";
    param_count_tags->chinese_explain = "统计标签个数";   
    UcdParamOpt::add_param(param_count_tags);
    
    // count_files
    ParamInfo * param_count_files = new ParamInfo("count_files");
    param_count_files->group = "ucd_opt";
    param_count_files->grammar = "ucd count_files file_dir recursive(true|1|True|false|0|False)";
    param_count_files->english_explain = "statistics file by suffix";
    param_count_files->chinese_explain = "统计文件夹中各后缀的文件数";   
    UcdParamOpt::add_param(param_count_files);
    
    // cut_small_img
    ParamInfo * param_cut_small_img = new ParamInfo("cut_small_img");
    param_cut_small_img->group = "ucd_opt";
    param_cut_small_img->grammar = "ucd cut_small_img img_dir xml_dir save_dir is_split(true|1|True|false|0|False)";
    param_cut_small_img->english_explain = "cut img by dete obj";
    param_cut_small_img->chinese_explain = "裁剪出小图";   
    UcdParamOpt::add_param(param_cut_small_img);
    
    // crop_to_xml
    ParamInfo * param_crop_to_xml = new ParamInfo("crop_to_xml");
    param_crop_to_xml->group = "ucd_opt";
    param_crop_to_xml->grammar = "ucd crop_to_xml crop_dir, save_dir";
    param_crop_to_xml->english_explain = "cut img to xml";
    param_crop_to_xml->chinese_explain = "截图生成xml";   
    UcdParamOpt::add_param(param_crop_to_xml);

    // check_xml
    ParamInfo * param_check_xml = new ParamInfo("check_xml");
    param_check_xml->group = "ucd_opt";
    param_check_xml->grammar = "ucd check_xml xml_dir img_dir size_th remove_error(true|1|True|false|0|False)";
    param_check_xml->english_explain = "check if xml is format";
    param_check_xml->chinese_explain = "检查xml是否符合标准";   
    UcdParamOpt::add_param(param_check_xml);

    // format_xml
    ParamInfo * param_format_xml = new ParamInfo("format_xml");
    param_format_xml->group = "ucd_opt";
    param_format_xml->grammar = "ucd format_xml xml_dir {img_dir}";
    param_format_xml->english_explain = "format xml";
    param_format_xml->chinese_explain = "将 xml 进行标准化";   
    UcdParamOpt::add_param(param_format_xml);

    // say
    ParamInfo * param_say = new ParamInfo("say");
    param_say->group = "-";
    param_say->grammar = "";
    param_say->english_explain = "";
    param_say->chinese_explain = "";   
    UcdParamOpt::add_param(param_say);

    // filter
    ParamInfo * param_filter = new ParamInfo("filter");
    param_filter->group = "-";
    param_filter->grammar = "";
    param_filter->english_explain = "";
    param_filter->chinese_explain = "";   
    UcdParamOpt::add_param(param_filter);

    // cache
    ParamInfo * param_cache = new ParamInfo("cache");
    param_cache->group = "-";
    param_cache->grammar = "";
    param_cache->english_explain = "";
    param_cache->chinese_explain = "";   
    UcdParamOpt::add_param(param_cache);

    // to
    ParamInfo * param_to = new ParamInfo("to");
    param_to->group = "-";
    param_to->grammar = "";
    param_to->english_explain = "";
    param_to->chinese_explain = "";   
    UcdParamOpt::add_param(param_to);

    // acc
    ParamInfo * param_acc = new ParamInfo("acc");
    param_acc->group = "-";
    param_acc->grammar = "";
    param_acc->english_explain = "";
    param_acc->chinese_explain = "";   
    UcdParamOpt::add_param(param_acc);

    // gif
    ParamInfo * param_gif = new ParamInfo("gif");
    param_gif->group = "-";
    param_gif->grammar = "";
    param_gif->english_explain = "";
    param_gif->chinese_explain = "";   
    UcdParamOpt::add_param(param_gif);

    // attr
    ParamInfo * param_attr = new ParamInfo("attr");
    param_attr->group = "-";
    param_attr->grammar = "";
    param_attr->english_explain = "";
    param_attr->chinese_explain = "";   
    UcdParamOpt::add_param(param_attr);

    // help
    ParamInfo * param_help = new ParamInfo("help");
    param_help->group = "-";
    param_help->grammar = "ucd help command";
    param_help->english_explain = "print command info";
    param_help->chinese_explain = "打印指定 command 对应的信息";   
    UcdParamOpt::add_param(param_help);


}



