#ifndef __HTTPCLIENT_H__

#include <curl/curl.h>
#include <string>
#include <list>

class http_client
{
public:
	http_client();
	~http_client();
    bool get(const std::string& url, std::string& data);
    bool post(const std::string& url, const std::string& body, std::string& data, const std::list<std::string>& headers_list);
private:
	CURL* curl_;
};


#endif // !__HTTPCLIENT_H__
