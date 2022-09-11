#include"compile_run.hpp"
#include"../comm/httplib.h"

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cout << "Usage : " << proc << ":" << "port" << std::endl;
}
// 编译服务随时可能被多个人请求，必须保证传递上来的code，形成源文件的名称的时候，要具有唯一性，否则多个用户之间会互相影响
int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }
    //通过HTTP，让client给我们上传一个json string

    Server svr;
    svr.Post("/compile_and_run",[](const Request &req,Response &resp){
        //用户请求的服务正文是我们想要的json string
        std::string in_json = req.body;//正文
        std::string out_json;
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json,&out_json);
            resp.set_content(out_json,"application/json;charset=utf-8");
        }
    });
    svr.listen("0.0.0.0",atoi(argv[1]));













    // //测试http基本使用
    // Server svr;

    // svr.Get("/hello",[](const Request &req,Response &resp){
    //     resp.set_content("hello httplib,你好 httplib!","text/plain;charset=utf-8");
    // });
    // svr.listen("0.0.0.0",8080);












    //---------------------用来测试
    // std::string in_json;
    // Json::Value in_value;
    // //R"()", raw string
    // in_value["code"] = R"(#include<iostream>
    // int main(){
    //     std::cout << "你可以看见我了" << std::endl;
    //     return 0;
    // })";
    // in_value["input"] = "";
    // in_value["cpu_limit"] = 1;
    // in_value["mem_limit"] = 10240*3;
    // Json::FastWriter writer;
    // in_json = writer.write(in_value);
    // std::cout << in_json << std::endl;
    // //这个是将来给客户端返回的json串
    // std::string out_json;
    // CompileAndRun::Start(in_json, &out_json);

    // std::cout << out_json << std::endl;
    
    // return 0;
}
