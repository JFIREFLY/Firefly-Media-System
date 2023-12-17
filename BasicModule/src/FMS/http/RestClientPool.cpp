#include "RestClientPool.h"
#include "curl.h" 

static int OnDebug(CURL*, curl_infotype itype, char* pData, size_t size, void*)
{
    if (itype == CURLINFO_TEXT)
    {

    }
    else if (itype == CURLINFO_HEADER_IN)
    {
        printf("[HEADER_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_OUT)
    {
        printf("[HEADER_OUT]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_IN)
    {
        printf("[DATA_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_OUT)
    {
        printf("[DATA_OUT]%s\n", pData);
    }

    return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
    if (NULL == str || NULL == buffer)
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}


RestClientPool::RestClientPool() :m_bDebug(false)
{
}

RestClientPool::~RestClientPool()
{
    for (std::vector<void*>::iterator it = m_VectCurl.begin(); it != m_VectCurl.end(); ++it)
    {
        curl_easy_cleanup(*it);
    }
}


void* RestClientPool::GetCurl()
{
    CURL* curl = NULL;

    m_mutex.lock();

    if (m_VectCurl.size() > 0)
    {
        curl = m_VectCurl.back();
        m_VectCurl.pop_back();
    }

    m_mutex.unlock();

    if (curl == NULL)
    {
        curl = CreateCurl();
    }

    return curl;
}

void* RestClientPool::CreateCurl()
{
    CURL* curl = curl_easy_init();

    if (NULL == curl)
    {
        return NULL;

    }

    if (m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);

     /**
     * Pass a long. If set to 1, TCP keepalive probes will be sent. 
     * The delay and frequency of these probes can be controlled by the CURLOPT_TCP_KEEPIDLE and CURLOPT_TCP_KEEPINTVL options,
     * provided the operating system supports them. Set to 0 (default behavior) to disable keepalive probes (Added in 7.25.0).
     */

    //enable TCP keep-alive for this transfer
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    
    #if 0
    //keep-alive idle time to 120 seconds
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120);

    //interval time between keep-alive probes: 60 seconds
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 20);
    #else
     /**
     * Pass a long. Sets the delay, in seconds, that the operating system will wait while the connection is idle before sending keepalive probes.
     * Not all operating systems support this option. (Added in 7.25.0)
     */

    //keep-alive idle time to 120 seconds
    //Windows环境生效；Linux环境待测
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120);

    /**
    * Pass a long. Sets the interval, in seconds, that the operating system will wait between sending keepalive probes.
    * Not all operating systems support this option. (Added in 7.25.0)
    */

    //interval time between keep - alive probes : 60 seconds
    //Windows环境不生效；Linux环境待测
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60);
    #endif

    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    return curl;
}

void RestClientPool::PutCurl(void* curl)
{
    m_mutex.lock();

    m_VectCurl.push_back(curl);

    m_mutex.unlock();
}

int RestClientPool::Post(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse)
{
    int res = CURLE_FAILED_INIT;

    CURL* curl = GetCurl();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    curl_slist* pCurlSlist = NULL;
    pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: text/json; charset=utf-8");

    if (NULL == pCurlSlist)
    {
        PutCurl(curl);
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    //连接对方主机时的最长等待时间，此设置限制的是建立连接过程的时间，其它过程的时间不在控制范围
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    //整个cURL函数执行过程的最长等待时间，也就是说，这个时间是包含连接等待时间的
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

    res = curl_easy_perform(curl);
    curl_slist_free_all(pCurlSlist);

    PutCurl(curl);

    return res;
}

int RestClientPool::Post(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader)
{
	int res = CURLE_FAILED_INIT;

	CURL* curl = GetCurl();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}

	curl_slist* pCurlSlist = NULL;
	pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: text/json; charset=utf-8");

	for (std::map<std::string, std::string>::iterator it = mapHeader.begin(); it != mapHeader.end(); ++it)
	{
		char buf[128] = { 0 };
		sprintf(buf, "%s: %s", it->first.c_str(), it->second.c_str());

		pCurlSlist = curl_slist_append(pCurlSlist, buf);
	}

	if (NULL == pCurlSlist)
	{
		PutCurl(curl);
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    //连接对方主机时的最长等待时间，此设置限制的是建立连接过程的时间，其它过程的时间不在控制范围
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    //整个cURL函数执行过程的最长等待时间，也就是说，这个时间是包含连接等待时间的
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

	res = curl_easy_perform(curl);
	curl_slist_free_all(pCurlSlist);

	PutCurl(curl);

	return res;
}

int RestClientPool::Get(const std::string& strUrl, unsigned int timeout, std::string& strResponse)
{
    int res = CURLE_FAILED_INIT;

    CURL* curl = GetCurl();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    curl_slist* pCurlSlist = NULL;
    pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: text/json; charset=utf-8");

    if (NULL == pCurlSlist)
    {
        PutCurl(curl);
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    //连接对方主机时的最长等待时间，此设置限制的是建立连接过程的时间，其它过程的时间不在控制范围
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    //整个cURL函数执行过程的最长等待时间，也就是说，这个时间是包含连接等待时间的
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

    res = curl_easy_perform(curl);
    curl_slist_free_all(pCurlSlist);

    PutCurl(curl);

    return res;

}

int RestClientPool::Get(const std::string& strUrl, unsigned int timeout, std::map<std::string, std::string>& mapHeader, std::string& strResponse)
{
	int res = CURLE_FAILED_INIT;

	CURL* curl = GetCurl();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}

	curl_slist* pCurlSlist = NULL;
	pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: text/json; charset=utf-8");

	for (std::map<std::string, std::string>::iterator it = mapHeader.begin(); it != mapHeader.end(); ++it)
	{
		char buf[128] = { 0 };
		sprintf(buf, "%s: %s", it->first.c_str(), it->second.c_str());

		pCurlSlist = curl_slist_append(pCurlSlist, buf);
	}

	if (NULL == pCurlSlist)
	{
		PutCurl(curl);
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    //连接对方主机时的最长等待时间，此设置限制的是建立连接过程的时间，其它过程的时间不在控制范围
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    //整个cURL函数执行过程的最长等待时间，也就是说，这个时间是包含连接等待时间的
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

	res = curl_easy_perform(curl);
	curl_slist_free_all(pCurlSlist);

	PutCurl(curl);

	return res;
}
