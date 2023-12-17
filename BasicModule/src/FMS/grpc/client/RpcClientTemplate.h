#ifndef __RPC_CLIENT_TEMPLATE__
#define __RPC_CLIENT_TEMPLATE__

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "siot_channel.grpc.pb.h"
#include "siot_channel.pb.h"

namespace ClientTemplate {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------     NewStubTemplate接口   -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------


template <class T>
std::unique_ptr<T> NewStubTemplate(std::shared_ptr<grpc::Channel>&)
{
	return NULL;
}

template <>
std::unique_ptr<CMS::SIOT::SIOTChannel::Stub> NewStubTemplate<CMS::SIOT::SIOTChannel::Stub>(std::shared_ptr<grpc::Channel>& channel)
{
	return CMS::SIOT::SIOTChannel::NewStub(channel);
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------      AsyncTemplate接口    -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------


template <class t, class req, class rsp>
std::unique_ptr<::grpc::ClientAsyncReaderWriter<req, rsp>> AsyncTemplate(std::unique_ptr<t>&, ::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag)
{
	return NULL;
}

template <>
std::unique_ptr<::grpc::ClientAsyncReaderWriter<::CMS::SIOT::SIOTChangeRequest, ::CMS::SIOT::SIOTChannelResponse>> AsyncTemplate<CMS::SIOT::SIOTChannel::Stub, CMS::SIOT::SIOTChangeRequest, CMS::SIOT::SIOTChannelResponse>(std::unique_ptr<CMS::SIOT::SIOTChannel::Stub>& stub, ::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag)
{
	return stub->AsyncChanged(context, cq, tag);
}


}; //ClientTemplate
#endif