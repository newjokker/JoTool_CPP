
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

    // DeteRes* a = new DeteRes("/home/ldq/logs/assign_logs_dir/jibei_v2_liuyue_24_model_00119373652-145c-11ed-901f-485f991ea484/xml_res/19376b3a-145c-11ed-835b-485f991ea484.xml");

    // DeteRes a;
    // 
    // std::cout << a->operator[](0).get_name_str() << std::endl;

    // a.parse_xml_info("/home/ldq/input_dir/test_data/00fa186e8d4d6660b49ddef8a35a77de.xml");
    // a.print_for);

    // count_tags("/home/disk2/res");
    
    clock_t start, end;

    start = clock();    
    
    count_tags(xml_dir);

    end = clock();

    std::cout << "use time " << (double)(end-start)/CLOCKS_PER_SEC << std::endl;

	// cut_small_img(
    //     "/home/ldq/input_dir/del_test_all/南网香港", 
    //     "/home/ldq/input_dir/del_test_all/南网香港", 
    //     "/home/ldq/input_dir/del_test_all/crop", true);

    // std::cout << "end" << std::endl;

	return 1;
	
}

