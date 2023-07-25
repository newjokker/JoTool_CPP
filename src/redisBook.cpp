
#include <iomanip>
#include <iostream>
#include <vector>
#include "include/redisBook.hpp"
#include <hiredis/hiredis.h> 
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <map>
#include "include/pystring.h"
// #include <ncurses.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

#define ERROR_COLOR         "\x1b[1;31m"    // 红色
#define HIGHTLIGHT_COLOR    "\033[1;35m"    // 品红
#define WARNNING_COLOR      "\033[1;33m"    // 橙色
#define STOP_COLOR          "\033[0m"       // 复原


void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO); // disable buffer and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO; // enable buffer and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


RedisBook::RedisBook(std::string host, int port, std::string name)
{
    RedisBook::host     = host;
    RedisBook::port     = port;
    RedisBook::name     = name;
    int connect_status  = RedisBook::connect();
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

int RedisBook::menu_loop(std::string book_name)
{

    if(book_name == "" || book_name == "menu")
    {
        RedisBook::get_menu_info();
    }

    if(book_name != "find")
    {
        while(true)
        {
            if(book_name == "menu" || book_name == "")
            {
                std::cout << "enter book index or book name :";
                std::getline(std::cin, book_name);
            }
            
            if(book_name == "joke" || book_name == "0")
            {
                RedisBook::tell_joke();
                break;
            }
            else if(book_name == "customer" || book_name == "1")
            {
                break;
            }
            else if(book_name == "txkj" || book_name == "2")
            {
                RedisBook::learn_tx_cluture();
                break;
            }
            else if(book_name == "svn" || book_name == "3")
            {
                break;
            }
            else if(book_name == "offical" || book_name == "4")
            {
                break;
            }
            else if(book_name == "pingcode" || book_name == "5")
            {
                break;
            }
            else if(book_name == "document" || book_name == "6")
            {
                // 文件服务器直接维护在 111 服务器，提供查询和下载和上传的功能
                // 上传的时候注意不能重名

                break;
            }
            else if(book_name == "docker" || book_name == "7")
            {
                RedisBook::print_docker_info();
                break;
            }
            else if(book_name == "q")
            {
                break;
            }
            else
            {
                std::cout << "* 输入 book 不存在，重新输入，键入 q 结束" << std::endl;
                book_name = "";
            }
        }
    }
    else
    {
        // 查找模式
    }


}

int RedisBook::learn_tx_cluture()
{
    std::cout << "\033[2J\033[1;1H";                            // 清除页面并将光标移动到第一行第一列

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << ERROR_COLOR << "愿景，使命，价值观" << STOP_COLOR << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      愿景   ：成为国际一流的能源电力人工智能技术公司" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      使命   ：用人工智能技术保障能源电力安全生产" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      价值观 ：诚朴 笃行   创新 敏捷" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << ERROR_COLOR << "价值观详解" << STOP_COLOR << std::endl;
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

    std::cout << ERROR_COLOR << "两个提倡" << STOP_COLOR << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      不让老实人吃亏" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "      自然竞争，成就伟大" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    std::cout << ERROR_COLOR << "反对自由主义。何为自由主义：" << STOP_COLOR << std::endl;
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

int RedisBook::tell_joke()
{

    std::vector<std::string> joke_ids = RedisBook::get_joke_id_vector();
    int joke_size   = joke_ids.size(); 
    int index       = 0;


    char ch;
    bool quit = false;

    while (read(STDIN_FILENO, &ch, 1) == 1 && ch != 'q') 
    {
        
        enableRawMode();

        // 
        std::cout << "\033[2J\033[1;1H";
        std::cout << "index : " << index << std::endl;
        Joke *each_joke = new Joke();
        RedisBook::load_assign_joke(joke_ids[index], each_joke);
        each_joke->print_info();

        // 输出导航界面
        for(int i=0; i<15; i++)
        {
            std::cout << "" << std::endl;
        }
        std::cout << "q : quit      d : next      a : last    t : add tag   c : add command" << std::endl;
        
        for(int i=0; i<5; i++)
        {
            std::cout << "" << std::endl;
        }
        // 

        if(ch == 'q')
        {
            quit = true;
        }
        else if(ch == 'a')
        {
            index -= 1;
        }
        else if(ch == 'd')
        {
            index += 1;
        }
        else if(ch == 't')
        {
            disableRawMode();
            std::cout << "add tag : ";
            std::string tags;
            std::getline(std::cin, tags);
            each_joke->add_tag(tags);
            RedisBook::update_assign_joke(each_joke->id, each_joke);
        }
        else if(ch == 'c')
        {
            disableRawMode();
            std::cout << "add command : ";
            std::string command;
            std::getline(std::cin, command);

            std::cout << command << std::endl;

            each_joke->add_command(command, RedisBook::name);
            RedisBook::update_assign_joke(each_joke->id, each_joke);
        }
        else if(ch == ':')
        {
            // 跳到指定的个数和 id 
        }
        else
        {
            std::cout << "key should in (q, d, a,)" <<  ch << std::endl; 
        }

        if(index < 0){index = 0;}
        if(index > joke_size - 1){index = joke_size-1;}
        if(quit == true){break;}

    }

    disableRawMode();

}

int RedisBook::load_assign_joke(std::string joker_id, Joke *each_joke)
{
    each_joke->id = joker_id;

    // load content
    std::string command_content = "HGET joke_content " + joker_id;
    redisReply* reply_content = (redisReply*)redisCommand(RedisBook::client, command_content.c_str());    
    if (reply_content == NULL) 
    {
        std::cout << ERROR_COLOR << "Failed to execute command : " << command_content << STOP_COLOR << std::endl;
        return 0;
    }
    each_joke->content = reply_content->str;
    freeReplyObject(reply_content);

    // load tags
    std::string command_tags = "HGET joke_tags " + joker_id;
    redisReply* reply_tags = (redisReply*)redisCommand(RedisBook::client, command_tags.c_str());    
    if (reply_tags == NULL) 
    {
        std::cout << ERROR_COLOR << "Failed to execute command "  << STOP_COLOR << command_tags << std::endl;
        return 0;
    }
    if(reply_tags->str)
    {
        std::vector<std::string> tag_list = pystring::split(reply_tags->str, ",");
        std::set<std::string> tag_set(tag_list.begin(), tag_list.end());
        each_joke->tags =  tag_set;
    }
    freeReplyObject(reply_tags);

    // load command
    std::string command_command = "HGET joke_command " + joker_id;
    redisReply* reply_command = (redisReply*)redisCommand(RedisBook::client, command_command.c_str());    
    if (reply_tags == NULL) 
    {
        std::cout << ERROR_COLOR << "Failed to execute command"  << command_command << STOP_COLOR << std::endl;
        return 0;
    }

    if(reply_command->str)
    {
        each_joke->command = pystring::split(reply_command->str, "-+-");    
    }
    freeReplyObject(reply_command);

    return 1;
}

int RedisBook::update_assign_joke(std::string joke_id, Joke *each_joke)
{

    // content
    std::string command_content = "HSET joke_content " + joke_id + " " + each_joke->content; 
    redisReply* reply_content = (redisReply*)redisCommand(RedisBook::client, command_content.c_str());
    if (reply_content == NULL) {
        std::cout << "SET command failed: " << RedisBook::client->errstr << std::endl;
        redisFree(RedisBook::client);
        return 1;
    }
    freeReplyObject(reply_content);

    // tags
    std::vector<std::string> tags_vector(each_joke->tags.begin(), each_joke->tags.end());
    std::string command_tags = "HSET joke_tags " + joke_id + " " + pystring::join(",", tags_vector); 
    redisReply* reply_tags = (redisReply*)redisCommand(RedisBook::client, command_tags.c_str());
    if (reply_tags == NULL) {
        std::cout << "SET command failed: " << RedisBook::client->errstr << std::endl;
        redisFree(RedisBook::client);
        return 1;
    }
    freeReplyObject(reply_tags);

    // command
    // TODO: 中间不能出现空格，不然插入不成功
    std::string command_all = pystring::join("-+-", each_joke->command);
    std::string command_command = "HSET joke_command " + joke_id + " " +  pystring::replace(command_all, " ", "_"); 

    // std::cout << command_command.c_str() << std::endl;

    // redisReply* reply_command = (redisReply*)redisCommand(RedisBook::client, command_command.c_str());
    redisReply* reply_command = (redisReply*)redisCommand(RedisBook::client, command_command.c_str());
    if (reply_command == NULL) {
        std::cout << "SET command failed: " << RedisBook::client->errstr << std::endl;
        redisFree(RedisBook::client);
        throw command_command;
        // return 1;
    }
    freeReplyObject(reply_command);

    return 1;
}

int RedisBook::print_docker_info()
{
    
    std::map<std::string, std::string> print_map;

    print_map["- posrt_v2"]   = "docker run --gpus device=0 -p 111:111 -v /home/ldq/output_test:/usr/output_dir  -v /home/ldq/logs_dir:/usr/logs -d test:v0.0.1";
    print_map["- feihua"]     = "docker run --gpus device=0 -p 5001:5001 -v /etc/localtime:/etc/localtime:ro -v /home/logs:/v0.0.1/logs -d test:v0.0.1";
    print_map["- wuhan_pd"]   = "docker run --gpus device=0 -v /home/input_dir:/usr/input_picture -v /home/output_dir:/usr/output_dir -v /home/ldq/json_dir:/usr/input_picture_attach -v /home/ldq/logs:/v0.0.1/logs -d wuhan_ft:v0.0.1";
    
    
    print_map["* commit -c"]    = "docker commit -c 'CMD /v0.01/start_server_jibei.sh' contain_id test:v0.0.1";
    print_map["* build  -t"]    = "docker build -t test:v0.0.1 -f docker_file_path .";
    print_map["* save   -o"]    = "docker save -o image_id test:v0.0.1";
    print_map["* exec   -it"]   = "docker exec -it image_id bash";
    print_map["* load   -i"]    = "docker load -i image_id";
    
    print_map["[*] cp "]    = "docker cp <container_id>:<container_path> <host_path>";


    print_map["[CMD] "]     = "docker rm -f $(docker ps -aq)";


    // TODO: 删除所有的 容器 docker rm -f $(docker ps -aq --filter "ancestor=fwppt_model:v1.4.7")

    // TODO: 删除所有的 容器 docker rm -f $(docker ps -aq --filter "ancestor=fwppt_model:v1.4.7")

    // docker file 的写法
    // 指定 时区
    // 指定 commit 时候启动脚本
    // 指定 commit 可以不用指定启动脚本
    // docker commit -c "CMD /v0.01/start_server_jibei.sh"

    // 常用的 docker 命令
    // docker 启动命令各个参数的解析 

    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "" << std::endl;

    auto iter = print_map.begin();
    while(iter != print_map.end())
    {
        std::cout << HIGHTLIGHT_COLOR << std::setw(10) << std::left << iter->first << " : " << STOP_COLOR;
        std::cout << iter->second << std::endl;
        std::cout << "" << std::endl;
        iter++;
    }
    std::cout << "--------------------------------------------------------------------------------" << std::endl;

    return 1;
}

std::vector<std::string> RedisBook::get_joke_id_vector()
{
    std::vector<std::string> book_ids;

    // load content
    redisReply* reply_content = (redisReply*)redisCommand(RedisBook::client, "HKEYS joke_content");    
    if (reply_content == NULL) 
    {
        std::cout << "Failed to execute command" << std::endl;
    }
    if (reply_content->type == REDIS_REPLY_ARRAY) 
    {   
        for (int i = 0; i < reply_content->elements; i++) 
        {
            book_ids.push_back(reply_content->element[i]->str);
        }
    }
    freeReplyObject(reply_content);
    return book_ids;
}

void Joke::print_info()
{
    std::cout << std::endl;

    std::cout << HIGHTLIGHT_COLOR << "ID : " << STOP_COLOR << Joke::id << std::endl;
    std::cout << "" << std::endl;
    
    std::cout << HIGHTLIGHT_COLOR << "content : " << STOP_COLOR << std::endl;
    std::cout << "    " << pystring::ljust(Joke::content, 1000) << std::endl;
    std::cout << "" << std::endl;

    // tags 
    std::cout << HIGHTLIGHT_COLOR << "tags : " << STOP_COLOR << std::endl;
    std::cout << "    ";
    auto iter = Joke::tags.begin();
    while(iter != Joke::tags.end())
    {
        std::cout << iter->data() << ","; 
        iter++;
    }
    std::cout << std::endl;
    std::cout << std::endl;

    // command
    std::cout << HIGHTLIGHT_COLOR << "command : " << STOP_COLOR << std::endl;
    for(int i=0; i<Joke::command.size(); i++)
    {
        std::cout << "    " << Joke::command[i] << std::endl;
    }
    std::cout << std::endl;

    // std::cout << "-------------------------------------" << std::endl;

}

int Joke::add_tag(std::string tag)
{
    Joke::tags.insert(tag);
    return 1;
}

int Joke::add_command(std::string command, std::string writer)
{

    // Get current time as a time_point object
    auto now = std::chrono::system_clock::now();

    // Convert time_point to time_t
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert time_t to tm
    std::tm* timeInfo = std::localtime(&currentTime);

    // Create a stringstream object
    std::ostringstream timeStream;

    // Write the current time into the stringstream in the specified format
    timeStream << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");

    // Get the resulting string
    std::string timeString = timeStream.str();

    Joke::command.push_back(timeString + "[" + writer + "]-->" + command);

    return 1;
}

TodoList::TodoList(std::string host, int port, std::string name)
{
    TodoList::port = port;
    TodoList::host = host;
    TodoList::name = name;
    TodoList::book_name = "todo_list_" + name;
}

int TodoList::add_todo_info(std::string date, std::string todo_info)
{
    if(pystring::find(todo_info, "-+-") != -1)
    {
        std::cout << ERROR_COLOR << "* 记录信息中不得存在字符 -+- " << STOP_COLOR << std::endl;
        return -1;
    }

    // 登录 redis
    redisContext *client = redisConnect(TodoList::host.c_str(), TodoList::port);
    std::vector<std::string> todo_info_history = TodoList::get_todo_info(date);
    todo_info_history.push_back(todo_info);

    std::string command_command = "HSET " + TodoList::book_name + " " + date + " " +  pystring::join("-+-", todo_info_history); 
    // std::cout << command_command << std::endl; 
    redisReply* reply_command = (redisReply*)redisCommand(client, command_command.c_str());
    if (reply_command == NULL) 
    {
        std::cout << "SET command failed: " << client->errstr << std::endl;
        redisFree(client);
        throw command_command;
    }
    freeReplyObject(reply_command);
    redisFree(client);
    return 1;
}

std::vector<std::string> TodoList::get_todo_info(std::string date)
{
    redisContext *client = redisConnect(TodoList::host.c_str(), TodoList::port);
    std::string command_str = "HGET " + TodoList::book_name + " " + date; 
    // std::cout << command_str << std::endl;
    redisReply* reply = (redisReply*)redisCommand(client, command_str.c_str());
    std::vector<std::string> empty_info;


    if (reply == NULL) 
    {
        std::cout << ERROR_COLOR << "Failed to execute command : " << command_str << STOP_COLOR << std::endl;
        return empty_info;
    }

    if (reply->type == REDIS_REPLY_NIL)
    {
        return empty_info;
    }
    else
    {
        // std::cout << "reply : " << reply->str << std::endl;
        std::vector<std::string> todo_info = pystring::split(reply->str, "-+-");    // 以 -+- 分隔
        freeReplyObject(reply);
        redisFree(client);
        return todo_info;
    }
}

int TodoList::print_todo_info(std::string assign_date)
{
    std::cout << "-------------------------------------------------------" << std::endl;
    std::vector<std::string> todo_info = TodoList::get_todo_info(assign_date);

    if(todo_info.size() == 0)
    {
        std::cout << ERROR_COLOR << "empty" << STOP_COLOR << std::endl;
    }
    else
    {
        std::cout << WARNNING_COLOR << assign_date << " (" + TodoList::name + ")" << " : " << STOP_COLOR << std::endl;
        // std::cout << "" << std::endl;

        for(int i=0; i<todo_info.size(); i++)
        {
            std::cout << std::endl; 
            if(pystring::startswith(todo_info[i], "[done]"))
            {
                std::cout << HIGHTLIGHT_COLOR << "    [" << i+1 << "] " << todo_info[i].substr(6) << " [done]" << STOP_COLOR << std::endl;
            }
            else
            {
                std::cout << "    [" << i+1 << "] " << todo_info[i] << std::endl;
            }
        }
    }
    std::cout << "-------------------------------------------------------" << std::endl;
    return 1;
}

int TodoList::print_todo_info_assign_date_range(std::string assign_date, int data_range)
{
    std::cout << "-------------------------------------------------------" << std::endl;
    for(int day=data_range; day > -1; day--)
    {
        std::string new_date = TodoList::get_date("", 0-day);

        std::vector<std::string> todo_info = TodoList::get_todo_info(new_date);

        std::cout << "" << std::endl;
        std::cout << WARNNING_COLOR << new_date << " (" + TodoList::name + ")" << " : " << STOP_COLOR << std::endl;

        if(todo_info.size() == 0)
        {
            std::cout << ERROR_COLOR << "empty" << STOP_COLOR << std::endl;
        }
        else
        {
            // std::cout << "" << std::endl;

            for(int i=0; i<todo_info.size(); i++)
            {
                std::cout << std::endl; 
                if(pystring::startswith(todo_info[i], "[done]"))
                {
                    std::cout << HIGHTLIGHT_COLOR << "    [" << i+1 << "] " << todo_info[i].substr(6) << " [done]" << STOP_COLOR << std::endl;
                }
                else
                {
                    std::cout << "    [" << i+1 << "] " << todo_info[i] << std::endl;
                }
            }
        }
    }
    std::cout << "-------------------------------------------------------" << std::endl;

    return 1;
}

std::string TodoList::get_date(std::string assign_date, int offset_day)
{

    // FIXME: 设计的有问题，不过用的比较少，就先不修改了

    // Get current time as a time_point object
    auto now = std::chrono::system_clock::now();

    if(offset_day > 0)
    {
        now = now + std::chrono::hours(24 * abs(offset_day));
    }
    else
    {
        now = now - std::chrono::hours(24 * abs(offset_day));
    }

    // Convert time_point to time_t
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert time_t to tm
    std::tm* timeInfo = std::localtime(&currentTime);

    // Create a stringstream object
    std::ostringstream timeStream;

    // Write the current time into the stringstream in the specified format
    timeStream << std::put_time(timeInfo, "%Y-%m-%d");

    // Get the resulting string
    std::string timeString = timeStream.str();

    // 
    if(assign_date == "")
    {
        return timeString;
    }
    else if(assign_date.size() == 5)
    {
        bool valid = TodoList::is_valid_date(assign_date);
        if(valid)
        {
            // 只写日期默认是今年的信息
            timeString = timeString.substr(0, 4) + "-" + assign_date;
            return timeString;
        }
        else
        {
            return "";
        }
    }
    else
    {
        return assign_date;
    }
}

bool TodoList::is_valid_date(std::string assign_date)
{
    if(assign_date.size() != 10 && assign_date.size() != 5)
    {
        return false;
    }

    if(assign_date.size() == 10)
    {
        if(assign_date[4] != '-' || assign_date[7] != '-')
        {
            return false;
        }
    }
    else
    {
        if(assign_date[2] != '-')
        {
            return false;
        }
    }
    return true;
}

int TodoList::update_todo_info(std::string assign_date, std::vector<std::string> todo_vector)
{

    // 登录 redis
    redisContext *client = redisConnect(TodoList::host.c_str(), TodoList::port);
    std::string command_command;
    if(todo_vector.size() == 0)
    {
        command_command = "HDEL " + TodoList::book_name + " " + assign_date; 

    }
    else
    {
        command_command = "HSET " + TodoList::book_name + " " + assign_date + " " +  pystring::join("-+-", todo_vector); 
    }

    // std::cout << command_command << std::endl;

    redisReply* reply_command = (redisReply*)redisCommand(client, command_command.c_str());
    if (reply_command == NULL) 
    {
        std::cout << "SET command failed: " << client->errstr << std::endl;
        redisFree(client);
        throw command_command;
    }
    freeReplyObject(reply_command);
    redisFree(client);
    return 1;
}

int TodoList::delete_todo_info(std::string assign_date, int assign_index)
{
    std::vector<std::string> todo_info = get_todo_info(assign_date);

    if(assign_index == -1)
    {
        todo_info.clear();
    }
    else if(assign_index > 0 && assign_index < (todo_info.size() + 1))
    {
        todo_info.erase(todo_info.begin() + (assign_index - 1));
    }
    else
    {
        std::cout << ERROR_COLOR << "index 不合法, index 范围是 1 到 " << todo_info.size() << " 的自然数" << STOP_COLOR << std::endl;
        return -1;
    } 

    TodoList::update_todo_info(assign_date, todo_info);
    return 1;
}

int TodoList::finish_todo(std::string assign_date, int assign_index)
{
    std::vector<std::string> todo_info = get_todo_info(assign_date);

    if(assign_index == -1)
    {
        for(int i=0; i<todo_info.size(); i++)
        {
            if(!pystring::startswith(todo_info[i], "[done]"))
            {
                todo_info[i] = "[done]" + todo_info[i];
            }
        }
    }
    else if(assign_index > 0 && assign_index < (todo_info.size() + 1))
    {
        if(!pystring::startswith(todo_info[assign_index-1], "[done]"))
        {
            todo_info[assign_index-1] = "[done]" + todo_info[assign_index-1];
        } 
    }
    else
    {
        std::cout << ERROR_COLOR << "index 不合法, index 范围是 1 到 " << todo_info.size() << " 的自然数" << STOP_COLOR << std::endl;
        return -1;
    } 

    TodoList::update_todo_info(assign_date, todo_info);
    return 1;
}

int TodoList::undo_todo(std::string assign_date, int assign_index)
{
    std::vector<std::string> todo_info = get_todo_info(assign_date);

    if(assign_index == -1)
    {
        for(int i=0; i<todo_info.size(); i++)
        {
            if(pystring::startswith(todo_info[i], "[done]"))
            {
                todo_info[i] = todo_info[i].substr(6);
            }
        }
    }
    else if(assign_index > 0 && assign_index < (todo_info.size() + 1))
    {
        if(pystring::startswith(todo_info[assign_index-1], "[done]"))
        {
            todo_info[assign_index-1] = todo_info[assign_index-1].substr(6);
        }
    }    
    else
    {
        std::cout << ERROR_COLOR << "index 不合法, index 范围是 1 到 " << todo_info.size() << " 的自然数" << STOP_COLOR << std::endl;
        return -1;
    } 

    TodoList::update_todo_info(assign_date, todo_info);
    return 1;
}







