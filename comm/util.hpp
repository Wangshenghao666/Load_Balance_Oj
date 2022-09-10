#pragma once
#include <string>
#include <vector>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/time.h>
#include<atomic>             //C++11中提供一个计数器，可以让我们定义一个原子性递增的计数器
#include<fstream>
#include<vector>
#include<boost/algorithm/string.hpp>

namespace ns_util
{
     //时间工具类
    class TimeUtil
    {
    public:
        //获得秒时间戳
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time,nullptr);
            return std::to_string(_time.tv_sec);
        }
        //获得毫秒时间戳
         static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    const std::string temp_path = "./temp/";
    // 路径工具类
    class PathUtil
    {
    public:
        //文件名+后缀
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }
        //构建源文件路径+后缀的完整文件名---eg:1234->./temp/1234.cpp

        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }

        //构建可执行程序的完整路径+后缀名---eg:1234->./temp/1234.exe
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }
        // 以上三个是编译时形成的文件

        // 以下是运行时形成的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name,".stdin");
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name,".stdout");
        }
        //构建该程序对应的标准错误完整路径+后缀名---eg:1234->./temp/1234.stderr
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name,".stderr");
        }
    };

    //文件工具类
    class FileUtil
    {
    public:
        //判断文件是否成功
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if(stat(path_name.c_str(),&st) == 0)
            {
                //获取属性成功，文件已经存在
                return true;
            }
            return false;
        }
        //形成具有唯一文件名
        static std::string UniqFileName()
        {
            static std::atomic_uint id(0);
            id++;
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }
        //写文件
        static bool WriteFile(const std::string &target,const std::string &content)
        {
            std::ofstream out(target);
            if(!out.is_open())
            {
                return false;
            }

            out.write(content.c_str(),content.size());
            out.close();
            return true;
        }
        //读文件
         static bool ReadFile(const std::string &target, std::string *content, bool keep = false)
        {
            (*content).clear();

            std::ifstream in(target);
            if (!in.is_open())
            {
                return false;
            }
            std::string line;
            // getline:不保存行分割符,有些时候需要保留\n,
            // getline内部重载了强制类型转化
            while (std::getline(in, line))
            {
                (*content) += line;
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };

    //字符串工具
    class StringUtil
    {
    public:
        //切分字符串
        /*************************************
         * str: 输入型，目标要切分的字符串
         * target: 输出型，保存切分完毕的结果
         * sep: 指定的分割符
         * **********************************/
        static void SplitString(const std::string &str,std::vector<std::string> *target,const std::string &sep)
        {
            boost::split((*target),str,boost::is_any_of(sep),boost::algorithm::token_compress_on);
        }
    };
}