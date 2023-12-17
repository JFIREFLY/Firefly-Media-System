#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_
#include "RestClientPool.h"

#include <string>
#include <map>

using namespace std;

class CHttpClient
{
public:
	CHttpClient();
	virtual ~CHttpClient();

	//∂Ã¡¥Ω”
	int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
	int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader);
	int Get(const std::string & strUrl, std::string & strResponse);
	int Get(const std::string& strUrl, std::map<std::string, std::string>& mapHeader, std::string& strResponse);
	int Get(const std::string& strUrl, std::string& strResponse, const std::string& strToken);
	int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);
	int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);

	//≥§¡¥Ω”
	int PostLongLink(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse);
	int PostLongLink(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader);
	int GetLongLink(const std::string& strUrl, unsigned int timeout, std::string& strResponse);
	int GetLongLink(const std::string& strUrl, unsigned int timeout, std::map<std::string, std::string>& mapHeader, std::string& strResponse);

public: 
	void SetDebug(bool bDebug);

private: 
	bool m_bDebug;
	RestClientPool m_clientPool;
};

#endif	//_HTTP_CLIENT_H_
