# 子任务


### 马慧娴

* 字符串格式化比较好的方法

```c++
std::string DeteObj::get_name_str()
{
    std::ostringstream oss;
    oss << "[" << DeteObj::x1 << "," << DeteObj::y1 << "," << DeteObj::x2 << "," << DeteObj::y2 << "," << DeteObj::conf << "," << DeteObj::tag << "]";
    return oss.str();
}
```

* 接口分析

* filter_by_mask()，返回 deteRes 比较好还是不返回直接对传入的数据进行修改比较好

* 


