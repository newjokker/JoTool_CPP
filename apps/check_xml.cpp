
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



int main_check_xml(int argc, char ** argv)
{

    // xml check

    // 直接打印结果就行了
    // 给一个选项可以设置是否一定文件到指定的文件夹，一定文件夹不是一个好的选项

    if ((argc!= 5))
    {
        std::cout << "need parameter number : 4 get : " << argc-1 << " {xml_dir, img_dir, size_th, if_remove_error_path}" << std::endl;
        return -1;
    }

    std::string xml_dir = argv[1];
    std::string img_dir = argv[2];
    int size_th = std::stoi(argv[3]);
    std::string remove_error_path = argv[4];
    
    bool remove_error = true;
    if((remove_error_path != "true") && (remove_error_path != "True") && (remove_error_path != "1"))
    {
        remove_error = false;
    }

    std::cout << "--------------------------------" << std::endl;
    std::cout << "xml dir           : " << xml_dir << std::endl;
    std::cout << "img_dir           : " << img_dir << std::endl;
    std::cout << "size_th           : " << size_th << std::endl;
    std::cout << "remove_error_file : " << remove_error << std::endl;
    std::cout << "--------------------------------" << std::endl;

    clock_t start, end;

    start = clock();
    
    xml_check(xml_dir, img_dir, size_th, remove_error);

    end = clock();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}
