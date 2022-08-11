
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
// #include "include/load_img.hpp"
#include "include/ucDatasetUtil.hpp"

using namespace jotools;
using namespace std;


// 将文件全部重命名
// 将对应的 xml 和 img 进行重命名

// 提供一个图床服务，图片专门放在一个服务器中 (放在多个服务器中，分担压力)
// 解析 json 下载对应的图片


int main(int argc, char ** argv)
{

    if ((argc <= 3))
    {
        std::cout << "need parameter number > 2 get : " << argc-1 << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "uc load json_path save_dir save_mode(111|100)" << std::endl;
        std::cout << "ucd check" << std::endl;
        std::cout << "ucd delete ucd_name" << std::endl;
        std::cout << "ucd upload ucd_path {ucd_name}" << std::endl;
        std::cout << "ucd load ucd_path" << std::endl;
        return -1;
    }


    // download_file("http://192.168.3.111:11101/file/Dsn06a6.xml", "/home/ldq/del/Dsn06a6.xml");

    
    UCDatasetUtil* ucd = new UCDatasetUtil("192.168.3.111" , 11101);
    ucd->json_path = "/home/ldq/del/test.json";

    // ucd->save_img_xml_json("/home/ldq/del/del_now");

    // ucd->check_ucd();

    ucd->delete_ucd("del_测试 (1)");


    ucd->save_ucd("nihao3", "/home/ldq/del/del_now/test.json");

	return 1;
	
}
