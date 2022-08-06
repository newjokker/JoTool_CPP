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
std::string get_file_name(std::string);
std::string get_file_name_suffix(std::string);

std::vector<std::string> get_all_file_path(std::string);
std::vector<std::string> get_all_folder_path(std::string);
std::vector<std::string> filter_by_suffix(std::vector<std::string>, std::set<std::string>);

FilePath parse_file_path(std::string);
void create_folder(std::string);


#endif