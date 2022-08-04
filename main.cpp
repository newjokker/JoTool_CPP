
#include <opencv2/opencv.hpp>
#include <iostream>
#include <set>
#include <time.h>
#include "./include/crow_all.h"
#include "./include/strToImg.hpp"
// #include "./include/yoloDete.hpp"
#include "./deteRes.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "./include/operateDeteRes.hpp"


int main()
{

    // DeteRes* a = new DeteRes("C:/Users/14271/Desktop/del/test_data/Dad012d.xml");

    DeteRes a;

    // a.parse_xml_info("/home/ldq/input_dir/test_data/00fa186e8d4d6660b49ddef8a35a77de.xml");
    // a.print_format();

    count_tags("/home/ldq/input_dir/test_data");
    // count_tags("/home/ldq/input_dir/xml");

	// cut_small_img(
    //     "/home/ldq/input_dir/test_data", 
    //     "/home/ldq/input_dir/test_data", 
    //     "/home/ldq/input_dir/test_data", true);

    // std::cout << "end" << std::endl;

	return 1;
	
}

