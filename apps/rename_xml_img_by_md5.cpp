
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


// 将文件全部重命名
// 将对应的 xml 和 img 进行重命名

// 提供一个图床服务，图片专门放在一个服务器中 (放在多个服务器中，分担压力)
// 解析 json 下载对应的图片


int main_xml_check(int argc, char ** argv)
{

    // xml check

    // 直接打印结果就行了
    // 给一个选项可以设置是否一定文件到指定的文件夹，一定文件夹不是一个好的选项

    if ((argc!= 3))
    {
        std::cout << "need parameter number : 2 get : " << argc-1 << " {xml_dir, img_dir}" << std::endl;
        return -1;
    }

    std::string xml_dir = argv[1];
    std::string img_dir = argv[2];

    std::cout << "--------------------------------" << std::endl;
    std::cout << "xml dir           : " << xml_dir << std::endl;
    std::cout << "img_dir           : " << img_dir << std::endl;
    std::cout << "--------------------------------" << std::endl;


    clock_t start, end;

    start = clock();
    
    rename_xml_img_by_md5(xml_dir, img_dir);

    end = clock();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}
