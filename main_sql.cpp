
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <mysql/mysql.h>
#include <stdlib.h>

using namespace std;


string g_Server = "192.168.3.101";  
string g_User = "root";         
string g_Password = "root123"; 
string g_Database = "Saturn_Database_beta";







void printVevtorVal(vector <string> strVal)
{
    vector<string>::iterator itr = strVal.begin();
    for(itr;itr!=strVal.end();itr++)
    {
        cout << (*itr) << " ";        
    }
    cout << endl;    
}

//vector二维表 输出
void printTableVal(vector <vector <string> > strVal)
{
    vector <vector <string> >::iterator itr = strVal.begin();
    for(itr; itr!=strVal.end(); itr++)
    {
        vector<string>::iterator tmp = (*itr).begin();
        for(tmp; tmp!=(*itr).end(); tmp++)
        {
            cout << setw(7) << left << (*tmp) << ", ";                
        }
        cout << endl;
    }
}

//查询数据库结果只有一个值
string getStrValSql(string strSql)
{
    string strvals = "";
    MYSQL *conn;
    MYSQL_RES *res;  
    conn = mysql_init(NULL);  

    if (!mysql_real_connect(conn, g_Server.c_str(),g_User.c_str(), g_Password.c_str(), g_Database.c_str(),0, NULL, 0))
    {
      return "";
    }     
    mysql_query(conn, strSql.c_str());       //执行sql语句
    res = mysql_store_result(conn);      //将查询结果装进MYSQL_RES
    if(!res)                                //sql执行结果判断
    {
        return "";
    }
    int rows = mysql_num_rows(res);      //获取结果行数        
    while(rows--)
    {
        MYSQL_ROW row = mysql_fetch_row(res); //从结果集中获取一行        
        strvals = (row[0] == NULL ? "":row[0]);        
    }
    mysql_free_result(res);   //查询完后记得要释放
    mysql_close(conn);
    
    return strvals;    
}

//数据库查询只有一行或者一列，返回vector
void getVecValSql(string strSql, vector<string> &vecVals)
{
    MYSQL *conn;
    MYSQL_RES *res;  
    conn = mysql_init(NULL);  

    if (!mysql_real_connect(conn, g_Server.c_str(),g_User.c_str(), g_Password.c_str(), g_Database.c_str(),0, NULL, 0))
    {
      return ;
    }     
    mysql_query(conn, strSql.c_str());       
    res = mysql_store_result(conn);      
    if(!res)                                
    {
        return ;
    }
    int rows = mysql_num_rows(res);               
    
    if( rows == 1)
    {    
        int cols = mysql_num_fields(res);
        int intTmp = 0;
        MYSQL_ROW row = mysql_fetch_row(res);          
        while(intTmp < cols)        
        {
            vecVals.push_back(row[intTmp] == NULL ? "":row[intTmp]);    
            intTmp++;
        }
    }
    else
    {
        while(rows--)
        {
            MYSQL_ROW row = mysql_fetch_row(res);              
            vecVals.push_back(row[0] == NULL ? "":row[0]);        
        }                
    }
    mysql_free_result(res);   
    mysql_close(conn);   
}

//数据库查询返回二维表
void getTableValSql(string strSql,vector <vector <string> > &vecVals)
{
    MYSQL *conn;
    MYSQL_RES *res;  
    conn = mysql_init(NULL);  

    if (!mysql_real_connect(conn, g_Server.c_str(),g_User.c_str(), g_Password.c_str(), g_Database.c_str(),  0, NULL, 0))
    {
        std::cout << "connect error" << std::endl;
        return ;
    }     
    mysql_query(conn, strSql.c_str());       
    res = mysql_store_result(conn);      
    if(!res)                                
    {
        std::cout << "query error" << std::endl;
        return ;
    }
    int rows = mysql_num_rows(res);               
    int cols = mysql_num_fields(res);    
    while(rows--)
    {
        int intTmp = 0;
        vector <string> tmp;        
        
        MYSQL_ROW row = mysql_fetch_row(res);                      
        while(intTmp < cols)        
        {            
            tmp.push_back(row[intTmp] == NULL ? "":row[intTmp]);    
            intTmp++;
        }        
        vecVals.push_back(tmp);        
    }

    mysql_free_result(res);   
    mysql_close(conn);  
}


int main_sql() 
{

    vector < vector<string> > res;
    getTableValSql("SELECT uc FROM MD5_uc WHERE MD5 = '6a761388756dead5510c6314e154e833';", res);

    printTableVal(res);

}