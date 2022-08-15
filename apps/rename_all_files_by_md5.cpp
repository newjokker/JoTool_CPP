
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


int main(int argc, char ** argv)
{

    if ((argc!= 2))
    {
        std::cout << "need parameter number : 1 get : " << argc-1 << " {file_dir}" << std::endl;
        return -1;
    }

    std::string file_dir = argv[1];

    std::cout << "--------------------------------" << std::endl;
    std::cout << "file dir           : " << file_dir << std::endl;
    std::cout << "--------------------------------" << std::endl;


    clock_t start, end;

    start = clock();
    
    rename_all_files_by_md5(file_dir);

    end = clock();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}
