#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <cassert>
#include <vector>
#include <fstream>
#include <stdlib.h>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number;   //题目编号---唯一
        std::string title;    //题目的标题
        std::string star;     //题目难度(简单、中等、困难)
        int cpu_limit;        //题目的时间复杂度(s)
        int mem_limit;        //题目的空间复杂度(kb)
        std::string desc;     //题目的描述
        std::string header;   //题目预设给用户在线编辑器的代码
        std::string tail;     //题目的测试用例

    };

    const string questins_list = "./questions/questions.list";
    const string questins_path = "./questions/";//题目路径
    class Model
    {
    private:
        //题目  ： 题目细节
        unordered_map<string,Question> questions;
    public:
        Model()
        {
            assert(LoadQuestionList(questins_list));
        }
        ~Model()
        {}
    public:
        bool LoadQuestionList(const string &question_list)
        {
            //加载配置文件: questions/questions.list + 题目编号文件
            //打开文件
            ifstream in(question_list);
            if(!in.is_open())
            {
                LOG(FATAL) << " 加载题库失败,请检查是否存在题库文件" << "\n";
                return false;
            }

            //读取
            string line;
            while(getline(in,line))
            {
                vector<string> tokens;
                StringUtil::SplitString(line,&tokens," ");
                if(tokens.size() != 5)
                {
                    //说明这一行配置有问题，继续读取下一行
                    LOG(WARNING) << "加载部分题目失败, 请检查文件格式" << "\n";
                    continue;
                }
                //说明切分成功了
                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = atoi(tokens[3].c_str());//注意切分完成是字符串，需要转整数
                q.mem_limit = atoi(tokens[4].c_str());

                string path = questins_path;
                path += q.number;
                path += "/";

                FileUtil::ReadFile(path+"desc.txt", &(q.desc), true);
                FileUtil::ReadFile(path+"header.cpp", &(q.header), true);
                FileUtil::ReadFile(path+"tail.cpp", &(q.tail), true);

                questions.insert({q.number, q});
            }
            LOG(INFO) << "加载题库...成功!" << "\n";
            //关闭文件
            in.close();
            return true;
        }
        bool GetAllQuestions(vector<Question> *out)
        {
            if(questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败" << "\n";
                return false;
            }
            for(const auto &q : questions)
            {
                out->push_back(q.second);
            }
            return true;
        }
        bool GetOneQuestion(const std::string &number,Question *q)
        {
            const auto &iter = questions.find(number);
            if(iter == questions.end())
            {
                LOG(ERROR) << "用户获取题目失败, 题目编号: " << number << "\n";
                return false;
            }
            (*q) = iter->second;
            return true;
        }
    };
}
//根据题目list文件，加载所有题目信息到内存中
//model---主要用来和数据进行交互，对外提供数据访问接口