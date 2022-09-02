#ifndef _LABELMEOBJ_HPP_
#define _LABELMEOBJ_HPP_


#include <iostream>
#include <vector>
#include <set>


class LabelmeObj
{    
    public:
        std::string label;
        std::vector< std::vector<double> > points;
        std::string shape_type = "None";
        std::set<std::string> flags;

        // 置信度信息，这个数据还是很重要的，想想还是加上去了
        float conf;
        
        int group_id;
        
        // 打印 obj 信息
        virtual void print_info() = 0;
        
        // 判断指针指向的对象是否相等
        virtual bool equal_to(LabelmeObj *obj) = 0;

};



class PointObj : public LabelmeObj
{
    public:
        PointObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class LineObj : public LabelmeObj
{
    public:
        LineObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class LineStripObj : public LabelmeObj
{
    public:
        LineStripObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class CircleObj : public LabelmeObj
{
    public:
        CircleObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class RectangleObj : public LabelmeObj
{
    public:
        RectangleObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class PolygonObj : public LabelmeObj
{
    public:
        PolygonObj();
        void print_info();
        bool equal_to(LabelmeObj *obj);
};

class LabelmeObjFactory
{
    public:
        LabelmeObj *CreateObj(std::string shape_type);

};




#endif