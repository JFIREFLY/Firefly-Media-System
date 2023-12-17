#ifndef _ISERVICE_H_
#define _ISERVICE_H_

class IService
{
public:
	virtual ~IService() {}

	virtual int Start() = 0;
	virtual int Stop() = 0;

};

#endif // _ISERVICE_H_
