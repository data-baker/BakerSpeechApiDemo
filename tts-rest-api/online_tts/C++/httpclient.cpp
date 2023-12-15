#include "httpclient.h"
#include <iostream>

using namespace std;

size_t req_reply(void* ptr, size_t size, size_t nmemb, string* reply)
{
	int totalSize = size * nmemb;
	reply->append((char*)ptr, (char*)ptr + totalSize);
	return totalSize;
}

http_client::http_client()
{
	curl_ = curl_easy_init();
}

http_client::~http_client()
{
	if (curl_ != NULL) {
		curl_easy_cleanup(curl_);
		curl_ = NULL;
	}
}

bool http_client::get(const std::string& url, std::string& data)
{
	string reply;
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30);//设置超时时间
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, req_reply);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)&reply);
	CURLcode res = curl_easy_perform(curl_);
	if (res != CURLE_OK) {
		curl_slist_free_all(headers);
		std::cout << "curl perform fail" << std::endl;
		return false;
	}
    data = reply;
    curl_slist_free_all(headers);
	return true;
}

bool http_client::post(const std::string& url, const std::string& body, std::string& data, std::string& content_type)
{
    string reply;
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_, CURLOPT_POST, 1);
	curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
	curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30);//设置超时时间
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, req_reply);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)&reply);
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        std::cout << "curl perform fail" << std::endl;
        if (headers != NULL) {
            curl_slist_free_all(headers);
		}
        return false;
    }
    char* hdr = NULL;
    res = curl_easy_getinfo(curl_, CURLINFO_CONTENT_TYPE, &hdr);
    if ((res == CURLE_OK) && hdr)
        content_type = hdr;
    data = reply;
    if (headers != NULL) {
        curl_slist_free_all(headers);
    }
    return true;
}
