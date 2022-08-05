
#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include "./include/crow_all.h"
#include "./include/strToImg.hpp"
#include "./deteRes.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
// #include "./include/operateDeteRes.hpp"
#include "./include/pystring.h"
#include "./include/fileOperateUtil.hpp"

using namespace jotools;
using namespace std;

int main(int argc, char ** argv)
{

    if (argc!= 2){
        std::cout << "need parameter number : 1, get : " << argc-1 << std::endl;
        return -1;
    }

    std::string crop_dir = argv[1];


    std::map<std::string, std::vector<std::string>> xml_info_map;


    std::vector<std::string> folder_path_list = get_all_folder_path(crop_dir);

    std::cout << folder_path_list.size() << std::endl; 


    for(int i=0; i<folder_path_list.size(); i++)
    {

        std::vector<std::string> file_path_vector = get_all_file_path(folder_path_list[i]);

        std::cout << file_path_vector.size() << std::endl; 


        std::set<std::string> suffixs;
        suffixs.insert(".jpg");
        suffixs.insert(".JPG");
        suffixs.insert(".png");
        suffixs.insert(".PNG");

        std::vector<std::string> img_path_vector = filter_by_suffix(file_path_vector, suffixs);

        std::cout << img_path_vector.size() << std::endl; 

        for(int j=0; j<img_path_vector.size(); j++)
        {
            std::string file_name = get_file_name(img_path_vector[j]);
            std::vector<std::string> loc_str_list = pystring::split(file_name, "-+-");
            std::string loc_str = loc_str_list[loc_str_list.size()-1];
            std::string name_str = pystring::slice(loc_str, 1, -2);

            std::cout << file_name << std::endl;
        }

    }


	return 1;
}

