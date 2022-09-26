


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

    if(diff_index < 0.6)
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

    if(ParamInfo::demo.size() != 0)
    {
        std::cout << "   [demo]"  << std::endl;
        for(int i=0; i<ParamInfo::demo.size(); i++)
        {
            std::cout << "   " << ParamInfo::demo[i] << std::endl;
        }
    }
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

void UcdParamOpt::print_all_fun_chinese()
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
            std::cout << std::setw(4) << std::left << " " << param_info.chinese_explain << std::endl;
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
    param_check->group = "sync";
    param_check->grammar = "ucd check";
    param_check->english_explain = "get all ucd official|customer from server";
    param_check->chinese_explain = "查看服务器中所有 ucd 信息，包括官方 ucd 和 自定义 ucd";
    UcdParamOpt::add_param(param_check);
    
    // search
    ParamInfo * param_search = new ParamInfo("search");
    param_search->group = "sync";
    param_search->grammar = "ucd search keyword";
    param_search->english_explain = "get all ucd official|customer from server by keyword";
    param_search->chinese_explain = "根据关键字查看服务器中所有 ucd 信息，包括官方 ucd 和 自定义 ucd";
    UcdParamOpt::add_param(param_search);
    
    // save
    ParamInfo * param_save = new ParamInfo("save");
    param_save->group = "sync";
    param_save->grammar = "ucd save ucd_path save_dir save_mode(image,xml) {need_count}";
    param_save->english_explain = "load img|xml from server";
    param_save->chinese_explain = "从服务器下载 图片|标注 保存到指定文件夹";   
    UcdParamOpt::add_param(param_save);
    
    // save_cache
    ParamInfo * param_save_cache = new ParamInfo("save_cache");
    param_save_cache->group = "sync";
    param_save_cache->grammar = "ucd save_cache ucd_path save_mode(image,xml)";
    param_save_cache->english_explain = "load img|xml from server";
    param_save_cache->chinese_explain = "从服务器下载 图片|标注 保存到 ucd 缓存文件夹";   
    UcdParamOpt::add_param(param_save_cache);
    
    // parse_xml
    ParamInfo * param_parse_xml = new ParamInfo("parse_xml");
    param_parse_xml->group = "sync";
    param_parse_xml->grammar = "ucd parse_xml json_path save_dir";
    param_parse_xml->english_explain = "parse xml from json";
    param_parse_xml->chinese_explain = "从 ucd 中解析出 xml";   
    UcdParamOpt::add_param(param_parse_xml);
    
    // parse_json
    ParamInfo * param_parse_json = new ParamInfo("parse_json");
    param_parse_json->group = "sync";
    param_parse_json->grammar = "ucd parse_xml json_path save_dir";
    param_parse_json->english_explain = "parse labelme_json from ucd";
    param_parse_json->chinese_explain = "从 ucd 中解析出 labelme_json";   
    UcdParamOpt::add_param(param_parse_json);
    
    // update_xml
    ParamInfo * param_update_xml = new ParamInfo("update_xml");
    param_update_xml->group = "sync";
    param_update_xml->grammar = "ucd update_xml {ucd_path}";
    param_update_xml->english_explain = "update cache xml";
    param_update_xml->chinese_explain = "更新本地缓存中的 xml, 可以全部更新，或者指定更新某 ucd 中包含的 uc 的本地 xml 缓存";   
    UcdParamOpt::add_param(param_update_xml);
    
    // show
    ParamInfo * param_show = new ParamInfo("show");
    param_show->group = "info";
    param_show->grammar = "ucd show {uc}";
    param_show->english_explain = "show for all url supported";
    param_show->chinese_explain = "查看所有下载路径";   
    UcdParamOpt::add_param(param_show);
    
    // delete
    ParamInfo * param_delete = new ParamInfo("delete");
    param_delete->group = "sync";
    param_delete->grammar = "ucd show {uc}";
    param_delete->english_explain = "ucd delete ucd_name";
    param_delete->chinese_explain = "删除在线数据集,无法删除官方数据集";   
    UcdParamOpt::add_param(param_delete);
    
    // load
    ParamInfo * param_load = new ParamInfo("load");
    param_load->group = "sync";
    param_load->grammar = "ucd load ucd_name save_path|save_dir";
    param_load->english_explain = "load ucd from server";
    param_load->chinese_explain = "下载在线数据集";   
    UcdParamOpt::add_param(param_load);
    
    // upload
    ParamInfo * param_upload = new ParamInfo("upload");
    param_upload->group = "sync";
    param_upload->grammar = "ucd upload ucd_path {ucd_name}";
    param_upload->english_explain = "upload ucd to server";
    param_upload->chinese_explain = "上传数据集到网络";   
    UcdParamOpt::add_param(param_upload);
    
    // from_img
    ParamInfo * param_from_img = new ParamInfo("from_img");
    param_from_img->group = "sync";
    param_from_img->grammar = "ucd from_img img_dir ucd_save_path";
    param_from_img->english_explain = "get ucd from loacal data(img)";
    param_from_img->chinese_explain = "(递归)本地文件生成数据集";   
    UcdParamOpt::add_param(param_from_img);
    
    // from_xml
    ParamInfo * param_from_xml = new ParamInfo("from_xml");
    param_from_xml->group = "sync";
    param_from_xml->grammar = "ucd from_xml xml_dir ucd_save_path";
    param_from_xml->english_explain = "get ucd from loacal data(img, xml)";
    param_from_xml->chinese_explain = "(递归)本地 voc_xml 文件生成 ucd, ucd中包含 dete_obj 信息";   
    UcdParamOpt::add_param(param_from_xml);
    
    // from_json
    ParamInfo * param_from_json = new ParamInfo("from_json");
    param_from_json->group = "sync";
    param_from_json->grammar = "ucd from_json json_dir ucd_save_path";
    param_from_json->english_explain = "get ucd from loacal data(img, json)";
    param_from_json->chinese_explain = "(递归)本地 labelme_json 文件生成 ucd, ucd中包含 dete_obj 信息";   
    UcdParamOpt::add_param(param_from_json);
    
    // info
    ParamInfo * param_info = new ParamInfo("info");
    param_info->group = "info";
    param_info->grammar = "ucd info ucd_path";
    param_info->english_explain = "show ucd info";
    param_info->chinese_explain = "查看数据集信息";   
    UcdParamOpt::add_param(param_info);
    
    // meta
    ParamInfo * param_meta = new ParamInfo("meta");
    param_meta->group = "info";
    param_meta->grammar = "ucd meta {attr_name}";
    param_meta->english_explain = "show config info";
    param_meta->chinese_explain = "查看配置信息, 可以指定配置的名字";   
    UcdParamOpt::add_param(param_meta);
    
    // set
    ParamInfo * param_set = new ParamInfo("set");
    param_set->group = "info";
    param_set->grammar = "ucd set key value";
    param_set->english_explain = "update config info";
    param_set->chinese_explain = "设置配置信息";   
    UcdParamOpt::add_param(param_set);
    
    // param_merge
    ParamInfo * param_merge = new ParamInfo("merge");
    param_merge->group = "opt";
    param_merge->grammar = "ucd merge save_path ucd_path1 ucd_path2 ...";
    param_merge->english_explain = "merge ucd info";
    param_merge->chinese_explain = "合并 ucd, 合并 uc_list label_used object_info 其他信息清空";   
    UcdParamOpt::add_param(param_merge);
    
    // minus
    ParamInfo * param_minus = new ParamInfo("minus");
    param_minus->group = "opt";
    param_minus->grammar = "ucd minus save_path ucd_path1 ucd_path2";
    param_minus->english_explain = "do minus operation between two ucd";
    param_minus->chinese_explain = "减操作数据集，目前不操作 object_info 信息";   
    UcdParamOpt::add_param(param_minus);
    
    // diff
    ParamInfo * param_diff = new ParamInfo("diff");
    param_diff->group = "info";
    param_diff->grammar = "ucd diff ucd_path1 ucd_path2";
    param_diff->english_explain = "compare two ucd";
    param_diff->chinese_explain = "比较数据集";   
    UcdParamOpt::add_param(param_diff);
    
    // rename_img
    ParamInfo * param_rename_img = new ParamInfo("rename_img");
    param_rename_img->group = "opt";
    param_rename_img->grammar = "ucd rename_img img_dir";
    param_rename_img->english_explain = "rename img by uc";
    param_rename_img->chinese_explain = "重命名数据集";   
    UcdParamOpt::add_param(param_rename_img);
    
    // rename_img_xml
    ParamInfo * param_rename_img_xml = new ParamInfo("rename_img_xml");
    param_rename_img_xml->group = "opt";
    param_rename_img_xml->grammar = "ucd rename_img_xml img_dir xml_dir";
    param_rename_img_xml->english_explain = "rename img xml by uc";
    param_rename_img_xml->chinese_explain = "重命名数据集";   
    UcdParamOpt::add_param(param_rename_img_xml);
    
    // rename_img_json
    ParamInfo * param_rename_img_json = new ParamInfo("rename_img_json");
    param_rename_img_json->group = "opt";
    param_rename_img_json->grammar = "ucd rename_img_json img_dir xml_dir";
    param_rename_img_json->english_explain = "rename img json by uc";
    param_rename_img_json->chinese_explain = "重命名 lableme json 数据集";   
    UcdParamOpt::add_param(param_rename_img_json);
    
    // count_tags
    ParamInfo * param_count_tags = new ParamInfo("count_tags");
    param_count_tags->group = "opt";
    param_count_tags->grammar = "ucd count_tags xml_dir|ucd_path";
    param_count_tags->english_explain = "count tags";
    param_count_tags->chinese_explain = "统计标签个数";   
    UcdParamOpt::add_param(param_count_tags);
    
    // count_files
    ParamInfo * param_count_files = new ParamInfo("count_files");
    param_count_files->group = "opt";
    param_count_files->grammar = "ucd count_files file_dir recursive(true|1|True|false|0|False)";
    param_count_files->english_explain = "statistics file by suffix";
    param_count_files->chinese_explain = "统计文件夹中各后缀的文件数";   
    UcdParamOpt::add_param(param_count_files);
    
    // cut_small_img
    ParamInfo * param_cut_small_img = new ParamInfo("cut_small_img");
    param_cut_small_img->group = "opt";
    param_cut_small_img->grammar = "ucd cut_small_img ucd_path save_dir is_split(true|1|True|false|0|False)";
    param_cut_small_img->english_explain = "cut img by dete obj";
    param_cut_small_img->chinese_explain = "裁剪出小图";   
    UcdParamOpt::add_param(param_cut_small_img);
    
    // crop_to_xml
    ParamInfo * param_crop_to_xml = new ParamInfo("crop_to_xml");
    param_crop_to_xml->group = "opt";
    param_crop_to_xml->grammar = "ucd crop_to_xml crop_dir, save_dir";
    param_crop_to_xml->english_explain = "cut img to xml";
    param_crop_to_xml->chinese_explain = "截图生成xml";   
    UcdParamOpt::add_param(param_crop_to_xml);

    // xml_check
    ParamInfo * param_xml_check = new ParamInfo("xml_check");
    param_xml_check->group = "check";
    param_xml_check->grammar = "ucd xml_check xml_dir img_dir size_th remove_error(true|1|True|false|0|False)";
    param_xml_check->english_explain = "check if xml is format";
    param_xml_check->chinese_explain = "检查xml是否符合标准";   
    UcdParamOpt::add_param(param_xml_check);

    // format_xml
    ParamInfo * param_format_xml = new ParamInfo("format_xml");
    param_format_xml->group = "opt";
    param_format_xml->grammar = "ucd format_xml xml_dir {img_dir}";
    param_format_xml->english_explain = "format xml";
    param_format_xml->chinese_explain = "将 xml 进行标准化";   
    UcdParamOpt::add_param(param_format_xml);

    // say
    ParamInfo * param_say = new ParamInfo("say");
    param_say->group = "fun";
    param_say->grammar = "ucd say words {height} {width}";
    param_say->english_explain = "say something with big character";
    param_say->chinese_explain = "将汉字放大说点什么";   
    UcdParamOpt::add_param(param_say);

    // filter
    ParamInfo * param_filter = new ParamInfo("filter");
    param_filter->group = "opt";
    param_filter->grammar = "ucd filter ucd_path save_path {filter_labes}";
    param_filter->english_explain = "filter ucd by labels";
    param_filter->chinese_explain = "对 ucd 进行过滤，可以指定过滤的标签，如果不指定，按照 ucd 中的 used_label 中的内容进行过滤";
    param_filter->demo = {
        "ucd filter test.json save_json",
        "ucd filter test.json save_json fzc_broken,Fnormal"};   
    UcdParamOpt::add_param(param_filter);

    // cache_info
    ParamInfo * param_cache_info = new ParamInfo("cache_info");
    param_cache_info->group = "cache";
    param_cache_info->grammar = "ucd cache_info {ucd_path}";
    param_cache_info->english_explain = "viewing cache info";
    param_cache_info->chinese_explain = "查看当前服务器的缓存信息";   
    UcdParamOpt::add_param(param_cache_info);
    
    // cache_clear
    ParamInfo * param_cache_clear = new ParamInfo("cache_clear");
    param_cache_clear->group = "cache";
    param_cache_clear->grammar = "ucd cache_clear {ucd_path}";
    param_cache_clear->english_explain = "clear cache";
    param_cache_clear->chinese_explain = "清空缓存信息";   
    UcdParamOpt::add_param(param_cache_clear);

    // to_yolo
    ParamInfo * param_to_yolo = new ParamInfo("to_yolo");
    param_to_yolo->group = "convert";
    param_to_yolo->grammar = "ucd to_yolo ucd_path save_dir {label_list}";
    param_to_yolo->english_explain = "convert ucd to yolo train txt (format)";
    param_to_yolo->chinese_explain = "将 ucd 转为 yolo txt 格式的数据";
    param_to_yolo->demo.push_back("ucd to_yolo fzc_test_1.json ./yolo_train_txt ");   
    param_to_yolo->demo.push_back("ucd to_yolo fzc_test_2.json ./yolo_train_txt Fnormal,fzc_broken");   
    UcdParamOpt::add_param(param_to_yolo);

    // // to_vit
    // ParamInfo * param_to_vit = new ParamInfo("to_vit");
    // param_to_vit->group = "convert";
    // param_to_vit->grammar = "";
    // param_to_vit->english_explain = "";
    // param_to_vit->chinese_explain = "";   
    // UcdParamOpt::add_param(param_to_vit);

    // // to_csra
    // ParamInfo * param_to_csra = new ParamInfo("to_csra");
    // param_to_csra->group = "convert";
    // param_to_csra->grammar = "";
    // param_to_csra->english_explain = "";
    // param_to_csra->chinese_explain = "";   
    // UcdParamOpt::add_param(param_to_csra);

    // // to_swin
    // ParamInfo * param_to_swin = new ParamInfo("to_swin");
    // param_to_swin->group = "convert";
    // param_to_swin->grammar = "";
    // param_to_swin->english_explain = "";
    // param_to_swin->chinese_explain = "转为 swintrnasform 训练需求的格式，具体实现的时候格式问张果容";   
    // UcdParamOpt::add_param(param_to_swin);

    // acc
    ParamInfo * param_acc = new ParamInfo("acc");
    param_acc->group = "opt";
    param_acc->grammar = "ucd acc ucd_customer, ucd_standard compare_res_ucd_path";
    param_acc->english_explain = "get acc rec from two ucd";
    param_acc->chinese_explain = "两个 ucd 之间计算精准率和召回率";   
    UcdParamOpt::add_param(param_acc);

    // gif
    ParamInfo * param_gif = new ParamInfo("gif");
    param_gif->group = "fun";
    param_gif->grammar = "";
    param_gif->english_explain = "";
    param_gif->chinese_explain = "";   
    UcdParamOpt::add_param(param_gif);

    // attr
    ParamInfo * param_attr = new ParamInfo("attr");
    param_attr->group = "sync";
    param_attr->grammar = "ucd attr ucd_path attr_name attr_value";
    param_attr->english_explain = "change attr in ucd";
    param_attr->chinese_explain = "修改ucd中使用 ucd info 能看到的属性";   
    UcdParamOpt::add_param(param_attr);

    // help
    ParamInfo * param_help = new ParamInfo("help");
    param_help->group = "info";
    param_help->grammar = "ucd help command";
    param_help->english_explain = "print command info";
    param_help->chinese_explain = "打印指定 command 对应的信息";   
    UcdParamOpt::add_param(param_help);

    // uc_check
    ParamInfo * param_uc_check = new ParamInfo("uc_check");
    param_uc_check->group = "check";
    param_uc_check->grammar = "ucd uc_check file_dir";
    param_uc_check->english_explain = "check if all file'name is uc";
    param_uc_check->chinese_explain = "查看是否所有文件以 UC 格式进行命名的";   
    UcdParamOpt::add_param(param_uc_check);

    // god blass me
    // todo 可以把保佑人的名字加在佛陀嘴边
    ParamInfo * param_god_bless = new ParamInfo("god_bless");
    param_god_bless->group = "fun";
    param_god_bless->grammar = "ucd god_bless {name}";
    param_god_bless->english_explain = "god bless";
    param_god_bless->chinese_explain = "上帝保佑";   
    UcdParamOpt::add_param(param_god_bless);

    // buddha blass me
    ParamInfo * param_buddha_bless = new ParamInfo("buddha_bless");
    param_buddha_bless->group = "fun";
    param_buddha_bless->grammar = "ucd buddha_bless {name}";
    param_buddha_bless->english_explain = "buddha bless";
    param_buddha_bless->chinese_explain = "佛陀保佑";   
    UcdParamOpt::add_param(param_buddha_bless);


    // move_uc
    ParamInfo * param_move_uc = new ParamInfo("move_uc");
    param_move_uc->group = "sync";
    param_move_uc->grammar = "ucd move_uc file_dir save_dir";
    param_move_uc->english_explain = "move file with name in uc format";
    param_move_uc->chinese_explain = "将文件件中所有符合 uc 命名的移动到指定文件夹中";   
    UcdParamOpt::add_param(param_move_uc);

    // move_not_uc
    ParamInfo * param_move_not_uc = new ParamInfo("move_not_uc");
    param_move_not_uc->group = "sync";
    param_move_not_uc->grammar = "ucd move_not_uc file_dir save_dir";
    param_move_not_uc->english_explain = "move file when filename not in uc format";
    param_move_not_uc->chinese_explain = "将文件件中所有不符合 uc 命名的移动到指定文件夹中";   
    UcdParamOpt::add_param(param_move_not_uc);

    // from_file
    ParamInfo * param_from_file = new ParamInfo("from_file");
    param_from_file->group = "sync";
    param_from_file->grammar = "ucd from_file file_dir";
    param_from_file->english_explain = "";
    param_from_file->chinese_explain = "(递归)只获取指定文件夹下面文件的 uc 组织成一个 ucd, 不去解析具体文件中的内容 ";   
    UcdParamOpt::add_param(param_from_file);

    // uc_analysis
    ParamInfo * param_uc_analysis = new ParamInfo("uc_analysis");
    param_uc_analysis->group = "info";
    param_uc_analysis->grammar = "ucd uc_analysis ucd_path";
    param_uc_analysis->english_explain = "";
    param_uc_analysis->chinese_explain = "分析 uc 的组成，看前三位即可";   
    UcdParamOpt::add_param(param_uc_analysis);

    // filter_by_conf
    ParamInfo * param_filter_by_conf = new ParamInfo("filter_by_conf");
    param_filter_by_conf->group = "filter";
    param_filter_by_conf->grammar = "ucd filter_by_conf ucd_path save_ucd_path conf_th";
    param_filter_by_conf->english_explain = "";
    param_filter_by_conf->chinese_explain = "对 ucd 进行指定阈值过滤";   
    UcdParamOpt::add_param(param_filter_by_conf);

    // filter_by_tags
    ParamInfo * param_filter_by_tags = new ParamInfo("filter_by_tags");
    param_filter_by_tags->group = "filter";
    param_filter_by_tags->grammar = "ucd filter_by_conf ucd_path save_ucd_path tag1,tag2,tag3";
    param_filter_by_tags->english_explain = "";
    param_filter_by_tags->chinese_explain = "对 ucd 指定标签过滤";   
    UcdParamOpt::add_param(param_filter_by_tags);

    // filter_by_nms
    ParamInfo * param_filter_by_nms = new ParamInfo("filter_by_nms");
    param_filter_by_nms->group = "filter";
    param_filter_by_nms->grammar = "ucd filter_by_nms ucd_path save_ucd_path nms_th ignore_tag(true|false)";
    param_filter_by_nms->english_explain = "do nms ";
    param_filter_by_nms->chinese_explain = "对 obj 进行 nms 操作";   
    UcdParamOpt::add_param(param_filter_by_nms);

    // conf_analysis
    ParamInfo * param_conf_analysis = new ParamInfo("conf_analysis");
    param_conf_analysis->group = "filter";
    param_conf_analysis->grammar = "ucd conf_analysis ucd_path seg_count";
    param_conf_analysis->english_explain = "";
    param_conf_analysis->chinese_explain = "置信度分析，分析 ucd 中存放的置信度信息";   
    UcdParamOpt::add_param(param_conf_analysis);

    // area_analysis
    ParamInfo * param_area_analysis = new ParamInfo("area_analysis");
    param_area_analysis->group = "filter";
    param_area_analysis->grammar = "ucd area_analysis ucd_path srg_count";
    param_area_analysis->english_explain = " ";
    param_area_analysis->chinese_explain = "面积分析，分析所有对象的面积分布";   
    UcdParamOpt::add_param(param_area_analysis);
    
    // get
    ParamInfo * param_get = new ParamInfo("get");
    param_get->group = "sync";
    param_get->grammar = "ucd get attr_name ucd_path";
    param_get->english_explain = "";
    param_get->chinese_explain = "获取 ucd 中的信息，[dataset_name, uc_count, label_used_count, model_name, model_version, add_time, update_time, describe, label_used, uc_list]";   
    param_get->demo.push_back("ucd get label_used aqm.json");
    UcdParamOpt::add_param(param_get);

    // drop
    ParamInfo * param_drop = new ParamInfo("drop");
    param_drop->group = "sync";
    param_drop->grammar = "ucd drop attr_name ucd_path save_ucd_path";
    param_drop->english_explain = "";
    param_drop->chinese_explain = "获取 ucd 中的信息 dataset_name, object_info, model_name, model_version, add_time, update_time, describe, label_used, uc_list]";   
    param_drop->demo.push_back("ucd drop object_info aqm.json aqm_drop_object_info.json");
    UcdParamOpt::add_param(param_drop);
    
    // has_uc
    ParamInfo * param_has_uc = new ParamInfo("has_uc");
    param_has_uc->group = "sync";
    param_has_uc->grammar = "ucd has_uc ucd_path assign_uc";
    param_has_uc->english_explain = "";
    param_has_uc->chinese_explain = "判断指定的 uc 是否在 ucd 的 uc_list 中";   
    param_has_uc->demo.push_back("ucd has_uc aqm.json Die11mk");
    UcdParamOpt::add_param(param_has_uc);
    
    // sub
    ParamInfo * param_sub = new ParamInfo("sub");
    param_sub->group = "opt";
    param_sub->grammar = "ucd sub ucd_path save_ucd_path need_count is_random";
    param_sub->english_explain = "";
    param_sub->chinese_explain = "从 ucd 中取出子序列，可以选择是否为随机选取";   
    param_sub->demo.push_back("ucd sub aqm.json aqm_sub.json 10 1");
    param_sub->demo.push_back("ucd sub aqm.json aqm_sub.json 10 0");
    UcdParamOpt::add_param(param_sub);

    // split
    ParamInfo * param_split = new ParamInfo("split");
    param_split->group = "opt";
    param_split->grammar = "ucd split ucd_path save_ucd_path_a save_ucd_path_a ratio";
    param_split->english_explain = "";
    param_split->chinese_explain = "将 ucd 按照比例划分为两个部分";   
    param_split->demo.push_back("ucd split aqm.json aqm_a.json aqm_b.json 0.2");
    param_split->demo.push_back("ucd split aqm.json aqm_a.json aqm_b.json 0.5");
    UcdParamOpt::add_param(param_split);
   
    // update
    ParamInfo * param_update = new ParamInfo("update");
    param_update->group = "sync";
    param_update->grammar = "ucd update {version}";
    param_update->english_explain = "";
    param_update->chinese_explain = "对 ucd 进行更新，只下载并配置库文件，还需要手动去改 ~/.bash_aliases 文件";   
    param_update->demo.push_back("ucd update");
    param_update->demo.push_back("ucd update ucd_v1.4.7");
    UcdParamOpt::add_param(param_update);
}

void UcdParamOpt::not_ready(std::string method_name)
{
    if(method_name == "")
    {
        std::cout << "----------------------------" << std::endl;
        std::cout << "* this method is not ready" << std::endl;
        std::cout << "----------------------------" << std::endl;
    }
    else
    {
        std::cout << "----------------------------" << std::endl;
        std::cout << "* method " << method_name << " is not ready" << std::endl;
        std::cout << "----------------------------" << std::endl;
    }

}

