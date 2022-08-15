
#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include "./include/crow_all.h"
#include "./include/strToImg.hpp"
#include "./deteRes.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "./include/operateDeteRes.hpp"

using namespace jotools;

int main_cut_small_img(int argc, char ** argv)
{

    if (argc!= 5){
        std::cout << "need parameter number : 4,  {xml_dir, img_dir, save_dir, is_split}, get : " << argc-1 << std::endl;
        return -1;
    }

    // get parameter
    std::string xml_dir = argv[1];
    std::string img_dir = argv[2];
    std::string save_dir = argv[3];
    std::string is_split_str = argv[4];

    bool is_split = true;
    if((is_split_str != "true") && (is_split_str != "True") && (is_split_str != "1"))
    {
        is_split = false;
    }

    // print
    std::cout << "xml dir : " << xml_dir << std::endl;
    std::cout << "img dir : " << img_dir << std::endl;
    std::cout << "save dir : " << save_dir << std::endl;
    std::cout << "is split : " << is_split << std::endl;

    clock_t start, end;
    start = clock();    
    
	cut_small_img(xml_dir, img_dir, save_dir, is_split);

    end = clock();
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;
	return 1;
	
}

