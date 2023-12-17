#pragma once

class ORpcBaseHandle
{
public:
	virtual int Start() = 0;
	virtual void Stop() = 0;
	virtual void Write(void* channel, void* rsp) = 0;
};
