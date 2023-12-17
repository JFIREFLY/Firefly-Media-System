#ifndef __RPC_STREAM_SERVER__
#define __RPC_STREAM_SERVER__

#include <string>
#include <list>
#include <grpcpp/server_builder.h>


//使用说明：
// 代码实现：
//1.在RpcServerTemplate.h中，特例化对应PB结构的RequestTemplate模板函数，函数实现固定，参照变量推送模板
//2.在RpcBusinessTemplate.h中，特例化对应PB结构的MsgCallbackTemplate模板函数，收到消息时会调用改函数，函数实现自定义
//3.在ORpcStreamServer.cpp中，注册对应PB结构的服务
//
// 用法：
// 实例化后调用Start函数


namespace RpcServer {

class ORpcStreamServer
{
public:
	ORpcStreamServer();
	~ORpcStreamServer();

	int Start(unsigned short port);
	void Stop();

private:
	void Register();

private:
	std::unique_ptr<grpc::Server> m_server;
	grpc::ServerBuilder m_builder;
	std::list<void*> m_handle_list;

private:
	std::mutex m_mutex;
};

}; //RpcServer

#endif