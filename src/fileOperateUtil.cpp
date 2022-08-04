

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
#include "../include/fileOperateUtil.hpp"



// 判断是否是一个文件
bool is_file(std::string filename) {
    struct stat   buffer;
    return (stat (filename.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

// 判断是否是一个文件夹,
bool is_dir(std::string filefodler) {
    struct stat   buffer;
    return (stat (filefodler.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}


std::string get_file_folder(std::string file_path)
{
    std::string folder = file_path.substr(0, file_path.find_last_of('/'));
    return folder;
}

std::string get_file_suffix(std::string file_path)
{
    std::string file_suffix = "." + file_path.substr(file_path.find_last_of('.') + 1);
    return file_suffix;
}

std::string get_file_name_suffix(std::string file_path)
{
    // std::string file_suffix = "." + file_path.substr(file_path.find_last_of('.') + 1);
    std::string file_name_suffix = file_path.substr(file_path.find_last_of('/') + 1);
    return file_name_suffix;
}

void GetFileNames(std::string path, std::vector<std::string>& filenames)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str())))
    {
        std::cout << "Folder doesn't Exist!" << std::endl;
        return;
    }
    while((ptr = readdir(pDir))!=0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
        {
            if(is_file(path + "/" + ptr->d_name))
            {
                filenames.push_back(path + "/" + ptr->d_name);
                // std::cout << "file file : " << path + "/" + ptr->d_name << std::endl;
            }
        }
    }
    closedir(pDir);
}

std::vector<std::string> filter_by_suffix(std::vector<std::string> filenames, std::set<std::string> suffixs)
{
    std::vector<std::string> new_file_names;
    for(int i=0; i<filenames.size(); i++)
    {
        std::string suffix = get_file_suffix(filenames[i]);
        if(suffixs.count(suffix) == 1)
        {
            new_file_names.push_back(filenames[i]);
        }
    }
    return new_file_names;
}

std::string get_file_name(std::string file_path)
{
    std::string file_suffix = "." + file_path.substr(file_path.find_last_of('.') + 1);
    std::string file_name_suffix = file_path.substr(file_path.find_last_of('/') + 1);
    std::string file_name = file_name_suffix.substr(0, file_name_suffix.length() - file_suffix.length());
    return file_name;
}

// 判断文件是否存在
inline bool if_file_exist (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

FilePath parse_file_path(std::string file_path_str)
{
    FilePath file_path;
    file_path.folder = get_file_folder(file_path_str);
    file_path.name = get_file_name(file_path_str);
    file_path.name_suffix = get_file_name_suffix(file_path_str);
    file_path.suffix = get_file_suffix(file_path_str);
    return file_path;
}

void create_folder(std::string folder_path)
{
    mkdir(folder_path.c_str(), S_IRWXU);
}