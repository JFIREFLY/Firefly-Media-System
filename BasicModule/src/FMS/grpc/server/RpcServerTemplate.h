#pragma once

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "siot_channel.grpc.pb.h"
#include "siot_channel.pb.h"

namespace ServerTemplate {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------     RequestTemplate�ӿ�   -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

//���������¼����а󶨵�����
template <class T>
void RequestTemplate(T* service, void* ctx, void* stream, void* new_cq, void* cq, void* tag)
{
	return;
}

//SIOT��������
template <>
void RequestTemplate<CMS::SIOT::SIOTChannel::AsyncService>(CMS::SIOT::SIOTChannel::AsyncService* service, void* ctx, void* stream, void* new_cq, void* cq, void* tag)
{
	service->RequestChanged((grpc::ServerContext*)ctx, (grpc::ServerAsyncReaderWriter<CMS::SIOT::SIOTChannelResponse, CMS::SIOT::SIOTChangeRequest>*)stream, (grpc::CompletionQueue*)new_cq, (grpc::ServerCompletionQueue*)cq, tag);
}


};
