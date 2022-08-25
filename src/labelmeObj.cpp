#include "include/lablelmeObj.hpp"


// 打印 obj 信息
void PointObj::print_info()
{
    std::cout << "label         : " << PointObj::label << std::endl;
    std::cout << "shape_type    : " << PointObj::shape_type << std::endl;
    std::cout << "points        : (" << PointObj::points[0][0] << ", " << PointObj::points[0][1] << ")" << std::endl;
}

void LineObj::print_info()
{
    std::cout << "label         : " << LineObj::label << std::endl;
    std::cout << "shape_type    : " << LineObj::shape_type << std::endl;
    std::cout << "points        : (" << LineObj::points[0][0] << ", " << LineObj::points[0][1] << "), ";
    std::cout << LineObj::points[1][0] << ", " << LineObj::points[1][1] << ")" << std::endl;
}

void LineStripObj::print_info()
{
    std::cout << "label         : " << LineStripObj::label << std::endl;
    std::cout << "shape_type    : " << LineStripObj::shape_type << std::endl;
    std::cout << "points_count  : " << LineStripObj::points.size() << std::endl;
}

void CircleObj::print_info()
{
    std::cout << "label         : " << CircleObj::label << std::endl;
    std::cout << "shape_type    : " << CircleObj::shape_type << std::endl;
    std::cout << "points        : (" << CircleObj::points[0][0] << ", " << CircleObj::points[0][1] << "), ";
    std::cout << CircleObj::points[1][0] << ", " << CircleObj::points[1][1] << ")" << std::endl;
}

void RectangleObj::print_info()
{
    std::cout << "label         : " << RectangleObj::label << std::endl;
    std::cout << "shape_type    : " << RectangleObj::shape_type << std::endl;
    // x1, x2, y1, y2
    std::cout << "points        : (" << RectangleObj::points[0][0] << ", " << RectangleObj::points[0][1] << ", ";
    std::cout << RectangleObj::points[1][0] << ", " << RectangleObj::points[1][1] << ")" << std::endl;
}

void PolygonObj::print_info()
{
    std::cout << "label         : " << PolygonObj::label << std::endl;
    std::cout << "shape_type    : " << PolygonObj::shape_type << std::endl;
    std::cout << "points_count  : " << PolygonObj::points.size() << std::endl;
}


// 判断是否相等
bool PointObj::equal_to(LabelmeObj* other_obj)
{
    if(PointObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(PointObj::label != other_obj->label)
    {
        return false;
    }

    if(PointObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<PointObj::points.size(); i++)
    {
        if((PointObj::points[i][0] != other_obj->points[i][0]) || (PointObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

bool LineObj::equal_to(LabelmeObj* other_obj)
{
    if(LineObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(LineObj::label != other_obj->label)
    {
        return false;
    }

    if(LineObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<LineObj::points.size(); i++)
    {
        if((LineObj::points[i][0] != other_obj->points[i][0]) || (LineObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

bool LineStripObj::equal_to(LabelmeObj* other_obj)
{
    if(LineStripObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(LineStripObj::label != other_obj->label)
    {
        return false;
    }

    if(LineStripObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<LineStripObj::points.size(); i++)
    {
        if((LineStripObj::points[i][0] != other_obj->points[i][0]) || (LineStripObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

bool CircleObj::equal_to(LabelmeObj* other_obj)
{
    if(CircleObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(CircleObj::label != other_obj->label)
    {
        return false;
    }

    if(CircleObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<CircleObj::points.size(); i++)
    {
        if((CircleObj::points[i][0] != other_obj->points[i][0]) || (CircleObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

bool RectangleObj::equal_to(LabelmeObj* other_obj)
{
    if(RectangleObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(RectangleObj::label != other_obj->label)
    {
        return false;
    }

    if(RectangleObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<RectangleObj::points.size(); i++)
    {
        if((RectangleObj::points[i][0] != other_obj->points[i][0]) || (RectangleObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

bool PolygonObj::equal_to(LabelmeObj* other_obj)
{
    if(PolygonObj::shape_type != other_obj->shape_type)
    {
        return false;
    }

    if(PolygonObj::label != other_obj->label)
    {
        return false;
    }

    if(PolygonObj::points.size() != other_obj->points.size())
    {
        return false;
    }

    for(int i=0; i<PolygonObj::points.size(); i++)
    {
        if((PolygonObj::points[i][0] != other_obj->points[i][0]) || (PolygonObj::points[i][1] != other_obj->points[i][1]))
        {
            return false;
        }
    }

    return true;
}

// 构造函数
PointObj::PointObj():LabelmeObj()
{
    PointObj::shape_type = "point";
}

LineObj::LineObj():LabelmeObj()
{
    LineObj::shape_type = "line";
}

LineStripObj::LineStripObj():LabelmeObj()
{
    LineStripObj::shape_type = "linestrip";
}

CircleObj::CircleObj():LabelmeObj()
{
    CircleObj::shape_type = "circle";
}

RectangleObj::RectangleObj():LabelmeObj()
{
    RectangleObj::shape_type = "rectangle";
}

PolygonObj::PolygonObj():LabelmeObj()
{
    PolygonObj::shape_type = "polygon";
}


// 工厂函数
LabelmeObj* LabelmeObjFactory::CreateObj(std::string shape_type)
{

    if(shape_type == "point")
    {
        LabelmeObj * obj = new PointObj();
        return obj;
    }
    else if(shape_type == "line")
    {
        return new LineObj();
    }
    else if(shape_type == "linestrip")
    {
        return new LineStripObj();
    }
    else if(shape_type == "circle")
    {
        return new CircleObj();
    }
    else if(shape_type == "rectangle")
    {
        return new RectangleObj();
    }
    else if(shape_type == "polygon")
    {
        return new PolygonObj();
    }
    else
    {
        std::cout << "unrecognize shape_type : " << shape_type << std::endl;
        throw "recognize shape_type : " + shape_type;
    }
}


