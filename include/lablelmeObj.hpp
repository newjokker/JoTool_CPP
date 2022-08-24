#ifndef _LABELMEOBJ_HPP_
#define _LABELMEOBJ_HPP_


#include <iostream>
#include <vector>
#include <set>


struct point
{
    // 点
    float x;
    float y;
};

class LabelmeObj
{    
    // labelme 中各种标注类型的基类
    public:
        std::string label;
        std::vector<point> points;
        std::string shape_type;
        // 不知道干什么用的，先抄 labelme 的结构
        int group_id;
        std::set<std::string> flags;

        // 打印 obj 信息
        void print_info();
};

class PointObj : public LabelmeObj
{
    // 点
    public:
        std::string shape_type = "point";
};

class LineObj : public LabelmeObj
{
    // 线（两个点）
    public:
        std::string shape_type = "line";
};

class LineStripObj : public LabelmeObj
{
    // 线（多个点）
    public:
        std::string shape_type = "linestrip";
};

class CircleObj : public LabelmeObj
{
    // 圆
    public:
        std::string shape_type = "circle";
};

class RectangleObj : public LabelmeObj
{
    // 矩形
    public:
        std::string shape_type = "rectangle";
};

class PolygonObj : public LabelmeObj
{
    // 多边形 - 旋转矩形属于多边形
    public:
        std::string shape_type = "polygon";
};



#endif