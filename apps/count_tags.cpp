
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



int main_count_tags(int argc, char ** argv)
{

    if (argc!= 2){
            std::cout << "need parameter number : 1 get : " << argc-1 << std::endl;
            return -1;
    }

    std::string xml_dir = argv[1];

    clock_t start, end;

    start = clock();    
    
    count_tags(xml_dir);

    end = clock();

    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	return 1;
	
}

