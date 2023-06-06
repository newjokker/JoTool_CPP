#ifndef _REDISBOOK_HPP
#define _REDISBOOK_HPP

#include <iostream>
#include <vector>
#include <hiredis/hiredis.h> 



class RedisBook
{
    
    public:

        // 客户端
        redisContext *client;
        std::string host    = "192.168.3.221";
        int port            = 6379;

        // 菜单
        std::vector<std::string> menu;

        // 构造函数
        RedisBook(std::string host, int port);

        // 连接 redis
        int connect();

        // 关闭链接
        int close();

        // 获取菜单信息
        int get_menu_info();

        // txkj 文化学习
        int learn_tx_cluture();

};












#endif