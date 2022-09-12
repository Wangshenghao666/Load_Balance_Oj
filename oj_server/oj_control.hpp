#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <cassert>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <algorithm>

#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "../comm/httplib.h"
#include "./oj_model.hpp"
#include "./oj_view.hpp"

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace httplib;
    using namespace ns_model;
    using namespace ns_view;

    //提供服务的主机
    class Machine
    {
    public:
        std::string ip;  //编译服务的ip
        int port;        //编译服务的port
        uint64_t load;   //编译服务的负载
        std::mutex *mtx; // mutex禁止拷贝的，使用指针
    public:
        Machine() 
            : ip(""), port(0), load(0), mtx(nullptr)
        {

        }
        ~Machine()
        {

        }
    public:
        // 提升主机负载
        void IncLoad()
        {
            if (mtx) mtx->lock();
            ++load;
            if (mtx) mtx->unlock();
        }
        // 减少主机负载
        void DecLoad()
        {
            if (mtx) mtx->lock();
            --load;
            if (mtx) mtx->unlock();
        }
        void ResetLoad()
        {
            if(mtx) mtx->lock();
            load = 0;
            if(mtx) mtx->unlock();
        }
        // 获取主机负载,没有太大的意义，只是为了统一接口
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx) mtx->lock();
            _load = load;
            if (mtx) mtx->unlock();

            return _load;
        }
    };


    const std::string service_machine = "./conf/service_machine.conf";
    //负载均衡模块
    class LoadBlance
    {
    private:
        // 可以给我们提供编译服务的所有的主机
        // 每一台主机都有自己的下标，充当当前主机的id
        std::vector<Machine> machines;

        // 所有在线的主机id
        std::vector<int> online;

        // 所有离线的主机id
        std::vector<int> offline;

        // 保证LoadBlance它的数据安全
        std::mutex mtx;
    public:
        LoadBlance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功" << "\n";
        }
        ~LoadBlance()
        {
            
        }
    public:
        bool LoadConf(const std::string &machine_conf)  //加载配置文件
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                LOG(FATAL) << " 加载：" << machine_conf << " 失败 " << "\n";
                return false;
            }
            std::string line;
            while(getline(in,line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line,&tokens,":");
                if (tokens.size() != 2)
                {
                    LOG(WARNING) << " 切分 " << line << " 失败" << "\n";
                    continue;
                }

                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }
            in.close();
            return true;
        }

        // id: 输出型参数
        // m : 输出型参数
        bool SmartChoice(int *id,Machine **m)
        {
            // 1. 使用选择好的主机(更新该主机的负载)
            // 2. 我们需要可能离线该主机
            mtx.lock();
            // 负载均衡的算法
            // 1. 随机数+hash
            // 2. 轮询+hash
            int online_num = online.size();
            if (online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " 所有的后端编译主机已经离线, 请运维的同事尽快查看" << "\n";
                return false;
            }
            // 通过遍历的方式，找到所有负载最小的机器
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for (int i = 1; i < online_num; i++)
            {
                uint64_t curr_load = machines[online[i]].Load();
                if (min_load > curr_load)
                {
                    min_load = curr_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            mtx.unlock();
            return true;
        }
        void OfflineMachine(int which)
        {
            mtx.lock();
            for(auto iter = online.begin(); iter != online.end(); iter++)
            {
                if(*iter == which)
                {
                    machines[which].ResetLoad();
                    //要离线的主机已经找到啦
                    online.erase(iter);
                    offline.push_back(which);
                    break; //因为break的存在，所有我们暂时不考虑迭代器失效的问题
                }
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {
            //我们统一上线，后面统一解决
            mtx.lock();
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            mtx.unlock();

            LOG(INFO) << "所有的主机有上线啦!" << "\n";
        }


         //for test
        void ShowMachines()
        {
             mtx.lock();
             std::cout << "当前在线主机列表: ";
             for(auto &id : online)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             std::cout << "当前离线主机列表: ";
             for(auto &id : offline)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             mtx.unlock();
        }
    };






    // 这是我们的核心业务逻辑的控制器
    class Control
    {
    private:
        Model _model;//提供后台数据
        View _view;//提供html渲染功能
        LoadBlance _load_blance;//核心负载均衡
    public:
        Control()
        {}
        ~Control()
        {}
    public:
        void RecoveryMachine()
        {
            _load_blance.OnlineMachine();
        }

        //根据题目数据构建网页
        //html 输出型参数
        bool AllQuestions(string *html)
        {
            bool ret = true;
            vector<struct Question> all;
            if(_model.GetAllQuestions(&all))
            {
                sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2){
                    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
                });

                //获取题目信息成功，将所有题目信息数据构建成网页
                _view.AllExpandHtml(all, html);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                return false;
            }
            return ret;
        }
        bool Question(const string &number,string *html)
        {
            bool ret = true;
            struct Question q;
            if(_model.GetOneQuestion(number,&q))
            {
                //获取指定题目信息成功，将指定题目信息数据构建成网页
                _view.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                return false;
            }
            return ret;
        }

        //id: ...
        //code: #include..
        //input: ""
        void Judge(const std::string &number,const std::string in_json,std::string *out_json)
        {
            //0.根据题目编号、直接拿到对应的题目细节
            struct Question q;
            _model.GetOneQuestion(number,&q);
            //1.in_json进行反序列化，得到题目的id，得到用户提交的源代码，input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();

            //2.重新拼接用户代码+测试用例代码，形成新的代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);

            //3.选择负载最低的主机
            // 规则: 一直选择，直到主机可用，否则，就是全部挂掉
            while(true)
            {
                int id = 0;
                Machine *m = nullptr;
                if(!_load_blance.SmartChoice(&id, &m))
                {
                    break;
                }

                // 4. 然后发起http请求，得到结果
                Client cli(m->ip, m->port);
                m->IncLoad();
                LOG(INFO) << " 选择主机成功, 主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 当前主机的负载是: " << m->Load() << "\n";
                if(auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8"))
                {
                    // 5. 将结果赋值给out_json
                    if(res->status == 200)
                    {
                        *out_json = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求编译和运行服务成功..." << "\n";
                        break;
                    }
                    m->DecLoad();
                }
                else
                {
                    //请求失败
                    LOG(ERROR) << " 当前请求的主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 可能已经离线"<< "\n";
                    _load_blance.OfflineMachine(id);
                    _load_blance.ShowMachines(); //仅仅是为了用来调试
                }
            }
        }
    };
} 