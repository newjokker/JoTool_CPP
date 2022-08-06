
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
#include "./include/pystring.h"
#include "./include/fileOperateUtil.hpp"

using namespace jotools;
using namespace std;

int main_crop_to_xml(int argc, char ** argv)
{

    if (argc!= 3){
        std::cout << "need parameter number : 2, get : " << argc-1 << "{xml_dir, save_dir}, attention : 路径后面不要加斜杠" << std::endl;
        return -1;
    }

    std::string crop_dir = argv[1];
    std::string save_dir = argv[2];


    jotools::get_xml_from_crop_img(crop_dir, save_dir);

	return 1;
}

