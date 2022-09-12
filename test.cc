// 功能测试

#include<iostream>
#include<sys/time.h>
#include<sys/resource.h>
#include<unistd.h>
#include<signal.h>
#include<string>

using namespace std;

// void handler(int signo)
// {
//     cout << " signal : " << signo << endl;
//     exit(1);
// }
// int main()
// {
//     // 资源不足，导致OS终止进程，是通过信号终止的
//     for(int i = 1; i <= 31;++i)
//     {
//         signal(i,handler);
//     }
//     // // 限制累计运行时长
//     // struct rlimit r;
//     // r.rlim_cur = 1;//限制累计时长为1s
//     // r.rlim_max = RLIM_INFINITY;//表示硬资源为无穷
//     // setrlimit(RLIMIT_CPU,&r);
//     // while(1)
//     // {

//     // }
//     // return 0;

//     // 限制累计空间大小
//     struct rlimit r;
//     r.rlim_cur = 1024*1024*40;
//     r.rlim_max = RLIM_INFINITY;
//     setrlimit(RLIMIT_AS,&r);
//     int count = 0;
//     while(1)
//     {
//         int *p = new int[1024*1024];
//         count++;
//         cout << " size_count : " << count << endl;
//         sleep(1);
//     }
// }


// //----> jsoncpp测试
// #include<jsoncpp/json/json.h>
// int main()
// {
//     // 序列化
//     //将结构化数据转换成为字符串
//     //Value是Json的一个中间类，可以填充KV值
//     Json::Value root;
//     root["code"] = "mycode";
//     root["user"] = "wsh";
//     root["age"] = "22";

//     Json::StyledWriter writer;
//     string str = writer.write(root);
//     cout << str << endl;
//     return 0;
// }


// //---->测试boost库中字符串切分
// #include <boost/algorithm/string.hpp>
// #include <vector>
// int main()
// {
//     const string str = "1 判断回文数 简单 1 30000";
//     vector<string> tokens;
//     const string sep = " ";
//     boost::split(tokens,str,boost::is_any_of(sep),boost::algorithm::token_compress_on);
//     for(int i = 0; i < tokens.size(); ++i)
//     {
//         cout << tokens[i] << endl;
//     }
//     return 0;
// }

//---->测试cetmplate
#include<ctemplate/template.h>
int main()
{
    std::string in_html = "./test.html";
    std::string value = "测试<--->测试";

    // 形成数据字典
    ctemplate::TemplateDictionary root("test"); //unordered_map<> test;
    root.SetValue("key", value);                //test.insert({});

    // 获取被渲染网页对象
    ctemplate::Template *tpl = ctemplate::Template::GetTemplate(in_html, ctemplate::DO_NOT_STRIP);

    // 添加字典数据到网页中
    std::string out_html;
    tpl->Expand(&out_html, &root);

    //完成了渲染
    std::cout << out_html << std::endl;
}
