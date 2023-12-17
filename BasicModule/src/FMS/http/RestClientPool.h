#ifndef _REST_CLIENT_POOL_H
#define _REST_CLIENT_POOL_H

#include <mutex>
#include <vector>
#include <map>

class RestClientPool
{
public:
	RestClientPool();
	virtual ~RestClientPool();

	int Get(const std::string& strUrl, unsigned int timeout, std::string& strResponse);
	int Get(const std::string& strUrl, unsigned int timeout, std::map<std::string, std::string>& mapHeader, std::string& strResponse);
	int Post(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse);
	int Post(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader);

private:
	void* GetCurl();
	void* CreateCurl();
	void PutCurl(void* curl);

private:
	bool m_bDebug;
	std::vector<void*> m_VectCurl;
	std::mutex m_mutex;
};

#endif	//_REST_CLIENT_POOL_H
