//#include "stdafx.h"
#include "HttpClient.h"
#include "curl.h" 

#ifdef _WIN32
#pragma comment (lib,"libcurl.lib")
#endif


CHttpClient::CHttpClient():m_bDebug(false) 
{
}

CHttpClient::~CHttpClient()
{
}


static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)  
{  
    if(itype == CURLINFO_TEXT)  
    {  
   
    }  
    else if(itype == CURLINFO_HEADER_IN)  
    {  
        printf("[HEADER_IN]%s\n", pData);  
    }  
    else if(itype == CURLINFO_HEADER_OUT)  
    {  
        printf("[HEADER_OUT]%s\n", pData);  
    }  
    else if(itype == CURLINFO_DATA_IN)  
    {  
        printf("[DATA_IN]%s\n", pData);  
    }  
    else if(itype == CURLINFO_DATA_OUT)  
    {  
        printf("[DATA_OUT]%s\n", pData);  
    }  

    return 0;  
}  
  
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);  
    if( NULL == str || NULL == buffer )  
    {  
        return -1;  
    }  
  
    char* pData = (char*)buffer;  
    str->append(pData, size * nmemb);  
    return nmemb;  
}  
  
int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }

    curl_slist* pCurlSlist = NULL;
    pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: application/json; charset=utf-8");

    if (NULL == pCurlSlist)
    {
        return CURLE_FAILED_INIT;
    }

    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
	curl_easy_setopt(curl, CURLOPT_POST, 1);  
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
	
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60);  
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 50);
	res = curl_easy_perform(curl); 
    curl_slist_free_all(pCurlSlist);
    curl_easy_cleanup(curl);  

    return res;
}  
  
int CHttpClient::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader)
{
    int res = CURLE_FAILED_INIT;

	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}

    curl_slist* pCurlSlist = NULL;
    pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: application/json; charset=utf-8");

    for (std::map<std::string, std::string>::iterator it = mapHeader.begin(); it != mapHeader.end(); ++it)
    {
        char buf[128] = { 0 };
        sprintf(buf, "%s: %s", it->first.c_str(), it->second.c_str());

        pCurlSlist = curl_slist_append(pCurlSlist, buf);
    }

    if (NULL == pCurlSlist)
    {
        return CURLE_FAILED_INIT;
    }

	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 50);
	res = curl_easy_perform(curl);
	curl_slist_free_all(pCurlSlist);
	curl_easy_cleanup(curl);

	return res;
}

int CHttpClient::Get(const std::string & strUrl, std::string & strResponse)  
{  
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }

    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    /** 
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。 
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。 
    */  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);  
    res = curl_easy_perform(curl);  
    curl_easy_cleanup(curl);  
	
    return res;  
}

int CHttpClient::Get(const std::string& strUrl, std::string& strResponse, const std::string& strToken)
{
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    //头部填充Token参数
    struct curl_slist* pCurlSlist = NULL;
    std::string para = "Authorization:Bearer ";
    para.append(strToken.c_str());
    pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: application/json; charset=utf-8");
    pCurlSlist = curl_slist_append(pCurlSlist, para.c_str());

    if (NULL == pCurlSlist)
    {
        return CURLE_FAILED_INIT;
    }

    if (m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_slist_free_all(pCurlSlist);
    curl_easy_cleanup(curl);

    return res;

}
  
int CHttpClient::Get(const std::string& strUrl, std::map<std::string, std::string>& mapHeader, std::string& strResponse)
{
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    //头部填充参数
	curl_slist* pCurlSlist = NULL;
	pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: application/json; charset=utf-8");

	for (std::map<std::string, std::string>::iterator it = mapHeader.begin(); it != mapHeader.end(); ++it)
	{
		char buf[128] = { 0 };
		sprintf(buf, "%s: %s", it->first.c_str(), it->second.c_str());

		pCurlSlist = curl_slist_append(pCurlSlist, buf);
	}

    if (NULL == pCurlSlist)
    {
        return CURLE_FAILED_INIT;
    }

    if (m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_slist_free_all(pCurlSlist);
    curl_easy_cleanup(curl);

    return res;
}

int CHttpClient::Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath)  
{  
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }

	//头部填充参数
	curl_slist* pCurlSlist = NULL;
	pCurlSlist = curl_slist_append(pCurlSlist, "Content-Type: application/json; charset=utf-8");

	if (NULL == pCurlSlist)
	{
		return CURLE_FAILED_INIT;
	}

    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pCurlSlist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {  
        //缺省情况就是PEM，所以无需设置，另外支持DER  
        //curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);  
    res = curl_easy_perform(curl); 
    curl_slist_free_all(pCurlSlist);
    curl_easy_cleanup(curl);

    return res;  
}  
  
int CHttpClient::Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath)  
{  
    int res = CURLE_FAILED_INIT;

    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }

    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);  
    res = curl_easy_perform(curl);  
    curl_easy_cleanup(curl); 

    return res;  
}

int CHttpClient::PostLongLink(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse)
{
    return m_clientPool.Post(strUrl, timeout, strPost, strResponse);
}

int CHttpClient::PostLongLink(const std::string& strUrl, unsigned int timeout, const std::string& strPost, std::string& strResponse, std::map<std::string, std::string>& mapHeader)
{
    return m_clientPool.Post(strUrl, timeout, strPost, strResponse, mapHeader);
}

int CHttpClient::GetLongLink(const std::string& strUrl, unsigned int timeout, std::string& strResponse)
{
    return m_clientPool.Get(strUrl, timeout, strResponse);
}

int CHttpClient::GetLongLink(const std::string& strUrl, unsigned int timeout, std::map<std::string, std::string>& mapHeader, std::string& strResponse)
{
    return m_clientPool.Get(strUrl, timeout, mapHeader, strResponse);
}

void CHttpClient::SetDebug(bool bDebug)  
{  
    m_bDebug = bDebug;  
}  
