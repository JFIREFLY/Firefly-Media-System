#include "ORpcStreamServer.h"
#include "ORpcServiceHandle.h"

RpcServer::ORpcStreamServer::ORpcStreamServer()
{
}

RpcServer::ORpcStreamServer::~ORpcStreamServer()
{
	Stop();
}

void RpcServer::ORpcStreamServer::Register()
{
	//SIOT变量推送服务
	ORpcBaseHandle* handle = new ORpcServiceHandle<CMS::SIOT::SIOTChannel::AsyncService, CMS::SIOT::SIOTChangeRequest, CMS::SIOT::SIOTChannelResponse>(&m_builder);
	m_handle_list.push_back(handle);

	//其他服务

}

int RpcServer::ORpcStreamServer::Start(unsigned short port)
{
	//1.绑定端口,启动keepalive机制
	m_builder.AddListeningPort("0.0.0.0:" + std::to_string(port), grpc::InsecureServerCredentials());

	m_builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 10 * 60 * 1000 /*10 min*/);
	m_builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 20 * 1000 /*20 sec*/);
	m_builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
	m_builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 10 * 1000 /*10 sec*/);

	//2.注册服务
	Register();

	//3.启动服务监听
	m_server = m_builder.BuildAndStart();
	if (m_server.get() == NULL)
	{
		return -2;
	}

	//4.启动事件监听
	for (std::list<void*>::iterator it = m_handle_list.begin(); it != m_handle_list.end(); ++it)
	{
		if (NULL == *it)
		{
			continue;
		}

		((ORpcBaseHandle*)(*it))->Start();
	}

	return 0;
}

void RpcServer::ORpcStreamServer::Stop()
{
	for (std::list<void*>::iterator it = m_handle_list.begin(); it != m_handle_list.end(); ++it)
	{
		if (NULL == *it)
		{
			continue;
		}

		((ORpcBaseHandle*)(*it))->Stop();
		delete ((ORpcBaseHandle*)(*it));
		*it = NULL;
	}

	m_handle_list.clear();

	if (m_server.get())
	{
		m_server->Shutdown();
		m_server.reset();
	}
}
