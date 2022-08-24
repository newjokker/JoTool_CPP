#include "include/lablelmeObj.hpp"


void LabelmeObj::print_info()
{
    std::cout << "label         : " << LabelmeObj::label << std::endl;
    std::cout << "shape_type    : " << LabelmeObj::shape_type << std::endl;
    std::cout << "points_count  : " << LabelmeObj::points.size() << std::endl;
}





