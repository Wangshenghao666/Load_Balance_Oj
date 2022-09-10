#pragma once
#include<iostream>
#include<string>
#include"util.hpp"

namespace ns_log
{
    using namespace ns_util;
    //日志等级
    enum
    {
        INFO,    //常规--没有错误
        DEBUG,   //调试--调试日志
        WARNING, //警告--不影响后续运行
        ERROR,   //错误--用户请求不能继续
        FATAL,   //重大错误--整个系统不能用了
    };
    inline std::ostream &Log(const std::string &level,const std::string &file_name,int line)
    {
        //添加日志等级
        std::string message = "[";
        message += level;
        message += "]";

        //添加报错文件名称
        message += "[";
        message += file_name;
        message += "]";

        //添加报错行
        message += "[";
        message += std::to_string(line);
        message += "]";

        //添加时间戳
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";

        //cout本质内部是包含缓冲区的
        std::cout << message;//不要endl进行刷新
        return std::cout;
    }
    //开放式日志
    #define LOG(level) Log(#level,__FILE__,__LINE__)
}