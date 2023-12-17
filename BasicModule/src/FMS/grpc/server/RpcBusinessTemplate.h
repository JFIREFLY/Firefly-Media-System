#pragma once
#include "ORpcBaseHandle.h"

#include "siot_channel.grpc.pb.h"
#include "siot_channel.pb.h"

namespace BusinessTemplate {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------     MsgCallbackTemplate接口  --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

//消息回调函数模板
//参数1: channel 通道句柄
//参数2: data PB请求数据
//参数3: context 服务句柄
template <class T>
void MsgCallbackTemplate(void* channel, T* data, void* context)
{
	return;
}

//SIOT变量推送消息回调函数
template <>
void MsgCallbackTemplate<CMS::SIOT::SIOTChangeRequest>(void* channel, CMS::SIOT::SIOTChangeRequest* data, void* context)
{
	printf("收到消息: %s %s %d \n", data->tagdata()[0].name().c_str(), data->tagdata()[0].data().c_str(), data->tagdata()[0].quality());

	CMS::SIOT::SIOTChannelResponse rsp;
	rsp.set_msg("success");
	rsp.set_code(CMS::SIOT::SIOTChannelResponseCode::Success);

	ORpcBaseHandle* obj = (ORpcBaseHandle*)context;

	obj->Write(channel, &rsp);
}


};