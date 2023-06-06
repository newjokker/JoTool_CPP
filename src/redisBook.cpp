
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
            std::cout << HIGHTLIGHT_COLOR << i/2 << ", " << std::setw(10) << std::setfill(' ') << std::left << reply->element[i]->str << STOP_COLOR << " : " ;
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
    
    std::cout << "\033[2J\033[1;1H";                        // 清除页面并将光标移动到第一行第一列

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << HIGHTLIGHT_COLOR << "愿景，使命，价值观" << STOP_COLOR << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      愿景   ：成为国际一流的能源电力人工智能技术公司" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      使命   ：用人工智能技术保障能源电力安全生产" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      价值观 ：诚朴 笃行   创新 敏捷" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << HIGHTLIGHT_COLOR << "价值观详解" << STOP_COLOR << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      诚朴：真诚质朴" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      笃行：切实履行" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      创新：挑战自我，勇于开拓，创新是公司生存的基础。" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      敏捷：快速响应，密切协作，敏捷是公司发展的保障。" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << HIGHTLIGHT_COLOR << "反对自由主义。何为自由主义：" << STOP_COLOR << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      小团体内不作原则上的争论。任其下去，求得和平亲热。" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      命令不服从；工作安排不服从；组织架构调整不服从，不要组织纪律。" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      向周围同事传递负能量。" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      在客户、合作伙伴那里，或在媒体上说公司、团队坏话，造成恶劣影响。" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    return -1;

}







