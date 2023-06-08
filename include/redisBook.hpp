#ifndef _REDISBOOK_HPP
#define _REDISBOOK_HPP

#include <iostream>
#include <vector>
#include <hiredis/hiredis.h> 
#include <map>
#include <set>

class Joke 
{
    public:
        std::string id;
        std::string content;
        std::set<std::string > tags;
        std::vector<std::string>command;    // 评论信息，评论时间-+-评论id-+-评论内容
        
        // 打印笑话
        void print_info();

        int add_tag(std::string tag);

        int add_command(std::string command, std::string writer);

};

class RedisBook
{
    
    public:

        // 客户端
        redisContext *client;
        std::string name    = "";
        std::string host    = "192.168.3.221";
        int port            = 6379;

        // 菜单
        std::vector<std::string> menu;

        // 构造函数
        RedisBook(std::string host, int port, std::string name="anonymity");

        // 连接 redis
        int connect();

        // 关闭链接
        int close();

        // 获取菜单信息
        int get_menu_info();

        // 主菜单循环
        int menu_loop(std::string book_name);

        // txkj 文化学习
        int learn_tx_cluture();

        // 加载指定 id 的笑话
        int load_assign_joke(std::string joker_id, Joke *each_joke);

        // 更新笑话的信息
        int update_assign_joke(std::string joke_id, Joke *each_joke);

        // 开始笑话
        int tell_joke();
        
        // 获取所有笑话的 id
        std::vector<std::string> get_joke_id_vector();

};






#endif