#include <iostream>

#include "ORpcStreamClient.h"

void event_callback(int events, void* context)
{
	printf("event[%d]\n", events);
};

void msg_callback(void* data, void* context)
{
	CMS::SIOT::SIOTChannelResponse* obj = (CMS::SIOT::SIOTChannelResponse*)data;

	printf("msg[%s] code[%d]\n", obj->msg().c_str(), obj->code());
};

int main()
{
	RpcClient::ORpcStreamClient<CMS::SIOT::SIOTChannel::Stub, CMS::SIOT::SIOTChangeRequest, CMS::SIOT::SIOTChannelResponse> aa("127.0.0.1", 8860);

	aa.SetMsgCallback(msg_callback, NULL);
	aa.SetEventCallback(event_callback, NULL);

	for (int i = 0; i < 100; ++i)
	{
		aa.Open();
		Sleep(500);
		aa.Close();
	}

	Sleep(500);
	aa.Open();

	Sleep(1000);

	do 
	{
		CMS::SIOT::SIOTChangeRequest req;
		auto it = req.add_tagdata();
		it->set_name("aaa");
		it->set_data("123");
		it->set_quality(CMS::SIOT::SIOTChangeTagQuality::Good);


		aa.Write(req);

		Sleep(1000);

	} while (1);


	return 0;
};