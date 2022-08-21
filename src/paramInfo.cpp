


#include <iostream>
#include <string>
#include <algorithm>
#include "include/paramInfo.hpp"

// using namespace std;



int min_edit_cost(std::string s1, std::string s2, int ic, int dc, int rc) 
{
    // s1, s2 两个字符串计算最小编辑代价，ic insert, dc delete, rc replace 
    int m=s1.length(),n=s2.length();
    std::vector<std::vector<int> > v(m+1, std::vector<int>(n+1,0));
    for(int i=0;i<m+1;i++) {
        for(int j=0;j<n+1;j++) {
            if(i==0 && j!=0) v[i][j]=j*ic;
            else if(i!=0 && j==0) v[i][j]=i*dc;
            else if(i==0 && j==0) v[i][j]=0;
            else if(s1[i-1]==s2[j-1]) v[i][j]=v[i-1][j-1];
            else v[i][j] = std::min(std::min(v[i-1][j]+dc,v[i][j-1]+ic),v[i-1][j-1]+rc);
        }
    }
    return v[m][n];
}



ParamInfo::ParamInfo(std::string command)
{
    ParamInfo::command = command;
}

bool ParamInfo::is_similar(std::string other_command)
{
    // 判断两个命令是否相似

    if(ParamInfo::command == other_command)
    {
        return true;
    }

    // 字符串的相似程度就是计算字符串的最小编辑代价
    int mec = min_edit_cost(ParamInfo::command, other_command, 1, 1, 1);
    float diff_index = (float)mec / (float)(ParamInfo::command.size() + other_command.size());

    std::cout << "diff index : " << diff_index << std::endl;

    if(diff_index < 0.25)
    {
        return true;
    }
    else
    {
        return false;
    }

}

//

ParamInfo UcdParamOpt::get_param(std::string command)
{
    if(UcdParamOpt::param_map.count(command) == 0)
    {
        std::cout << "command not register : " << command << std::endl;
        throw "command not register";
    }
    else
    {
        return UcdParamOpt::param_map[command];
    }
}





