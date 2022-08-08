
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

    if (argc!= 3){
            std::cout << "need parameter number : 2 get : " << argc-1 << std::endl;
            return -1;
    }

    std::string xml_dir = argv[1];
    std::string recursive = argv[2];

    bool is_recursive = true;
    if((recursive == "false") || (recursive == "False") || (recursive == "0"))
    {
        is_recursive = false;
    }

    std::cout << "--------------------------------" << std::endl;
    std::cout << "xml dir   : " << xml_dir << std::endl;
    std::cout << "recursive : " << is_recursive << std::endl;
    std::cout << "--------------------------------" << std::endl;


    clock_t start, end;

    start = clock();    
    
    count_files(xml_dir, is_recursive);

    end = clock();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}
