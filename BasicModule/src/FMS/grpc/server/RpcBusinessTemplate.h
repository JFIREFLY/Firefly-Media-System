#pragma once
#include "ORpcBaseHandle.h"

#include "siot_channel.grpc.pb.h"
#include "siot_channel.pb.h"

namespace BusinessTemplate {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------     MsgCallbackTemplate�ӿ�  --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

//��Ϣ�ص�����ģ��
//����1: channel ͨ�����
//����2: data PB��������
//����3: context ������
template <class T>
void MsgCallbackTemplate(void* channel, T* data, void* context)
{
	return;
}

//SIOT����������Ϣ�ص�����
template <>
void MsgCallbackTemplate<CMS::SIOT::SIOTChangeRequest>(void* channel, CMS::SIOT::SIOTChangeRequest* data, void* context)
{
	printf("�յ���Ϣ: %s %s %d \n", data->tagdata()[0].name().c_str(), data->tagdata()[0].data().c_str(), data->tagdata()[0].quality());

	CMS::SIOT::SIOTChannelResponse rsp;
	rsp.set_msg("success");
	rsp.set_code(CMS::SIOT::SIOTChannelResponseCode::Success);

	ORpcBaseHandle* obj = (ORpcBaseHandle*)context;

	obj->Write(channel, &rsp);
}


};