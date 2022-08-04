#ifndef _FILEOPERSTEUTIL_HPP_
#define _FILEOPERSTEUTIL_HPP_


#include <string>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <string.h>
#include <sys/stat.h>

// 文件路径
struct FilePath
{
    std::string folder;
    std::string name;
    std::string name_suffix;
    std::string suffix;
};


bool is_file(std::string);
bool is_dir(std::string);
std::string get_file_folder(std::string);
std::string get_file_suffix(std::string);
std::string get_file_name_suffix(std::string);
void GetFileNames(std::string, std::vector<std::string>&);
std::vector<std::string> filter_by_suffix(std::vector<std::string>, std::set<std::string>);
std::string get_file_name(std::string);
inline bool if_file_exist (const std::string&);
//
FilePath parse_file_path(std::string);
void create_folder(std::string);

#endif