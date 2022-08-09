
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



int main(int argc, char ** argv)
{

    // xml check

    // 找到没有 xml img 对应的数据
    // 找到 img 和 xml 中图像大小不一致的数据
    // 找到存在 dete_obj 过小的数据


    if (argc!= 5)
    {
            std::cout << "need parameter number : 4 get : " << argc-1 << "{xml_dir, img_dir, size_th, save_dir}" << std::endl;
            return -1;
    }

    std::string xml_dir = argv[1];
    std::string img_dir = argv[2];
    int size_th = std::stoi(argv[3]);
    std::string save_dir = argv[4];


    std::cout << "--------------------------------" << std::endl;
    std::cout << "xml dir   : " << xml_dir << std::endl;
    std::cout << "img_dir   : " << img_dir << std::endl;
    std::cout << "size_th   : " << size_th << std::endl;
    std::cout << "save_dir   : " << save_dir << std::endl;
    std::cout << "--------------------------------" << std::endl;


    clock_t start, end;

    start = clock();    
    
    xml_check(xml_dir, img_dir, size_th);

    end = clock();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}
