
#include <iomanip>
#include <iostream>
#include <vector>
#include "include/redisBook.hpp"
#include <hiredis/hiredis.h> 


#define ERROR_COLOR         "\x1b[1;31m"    // 红色
#define HIGHTLIGHT_COLOR    "\033[1;35m"    // 品红
#define WARNNING_COLOR      "\033[1;33m"    // 橙色
#define STOP_COLOR          "\033[0m"       // 复原


RedisBook::RedisBook(std::string host, int port)
{
    int connect_status = RedisBook::connect();
    if(connect_status == -1)
    {
        std::cout << ERROR_COLOR << "connect to redis fail" << STOP_COLOR << std::endl;
    }
    else
    {

    }
}

int RedisBook::get_menu_info()
{

    // redisReply* reply = (redisReply*)redisCommand(RedisBook::client, "HKEYS menu");
    redisReply* reply = (redisReply*)redisCommand(RedisBook::client, "HGETALL menu");
    
    if (reply == NULL) {
        std::cout << "Failed to execute command" << std::endl;
        return 1;
    }

    if (reply->type == REDIS_REPLY_ARRAY) 
    {   
        std::cout << "" << std::endl;
        for (int i = 0; i < reply->elements; i+=2) 
        {
            std::cout << i/2 << ", " << std::setw(10) << std::setfill(' ') << std::left << reply->element[i]->str << " : ";
            std::cout << reply->element[i+1]->str << std::endl;
            std::cout << "" << std::endl;
        }
    }

    freeReplyObject(reply);
}

int RedisBook::connect()
{
    RedisBook::client = redisConnect(RedisBook::host.c_str(), RedisBook::port);
    if(RedisBook::client->err)
    {
        redisFree(RedisBook::client);
        return -1;
    }
    else
    {
        return 1;
    }
}

int RedisBook::close()
{
    redisFree(RedisBook::client);
}

int RedisBook::learn_tx_cluture()
{

    // 主要内容

    // 各部分内容的解释

    // 


}







