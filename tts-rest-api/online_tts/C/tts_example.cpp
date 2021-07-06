#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <string.h>
#define MAX_HEADER_VALUE_LEN 1000

using RequestType = enum{
	TYPE_GET_TOKEN=0,
	TYPE_GET_AUDIO
};

using HttpReply = struct{
	RequestType type;
	std::string response;
	FILE * fp;
	int result;
};

using RequestParam =  struct{
	float		speed;
	int			volume;
	float		pitch;
	int			audiotype;
	int			rate;
	int			spectrum;
	int			spectrum_8k;
	int			interval;
	int			domain;
	std::string client_id;
	std::string client_secret;
	std::string text;
	std::string voice_name;
	std::string language;;
};

size_t req_reply(void *ptr, size_t size, size_t nmemb, HttpReply *reply)  
{
	if(reply->type == TYPE_GET_TOKEN)
	{
		int totalSize = size * nmemb;
		reply->response.append((char *)ptr, (char *)ptr + totalSize);
		return totalSize;
	}
	else if(reply->type == TYPE_GET_AUDIO)
	{
		if(reply->result != 0)
		{
			int totalSize = size * nmemb;
			reply->response.append((char *)ptr, (char *)ptr + totalSize);
			return totalSize;
		}
		else
		{
			if(reply->fp)
			{
				return fwrite(ptr, size, nmemb, reply->fp);
			}

		}
	}
}


static int search_header(const char *buffer, size_t len, const char *key, char *value) {
    size_t len_key = strlen(key);
    char header_key[len_key + 1];
    header_key[len_key] = '\0';
    memcpy(header_key, buffer, len_key);
    if (strcasecmp(key, header_key) == 0 && (len - len_key) < MAX_HEADER_VALUE_LEN) {
        int len_value = len - len_key;
        value[len_value - 1] = '\0';
        memcpy(value, buffer + len_key + 1, len_value);
        return 0;
    }
    return -1;

}

size_t header_callback(char *buffer, size_t size, size_t nitems, HttpReply * reply) {
    size_t len = size * nitems;
    char key[] = "Content-Type";
    char value[MAX_HEADER_VALUE_LEN];
    if (search_header(buffer, len, key, value) == 0) {
        if (strstr(value, "audio/") != NULL) {
            std::cout<<"content-type is audio"<<std::endl;
			reply->result = 0;
        } else {
            std::cout<<"content-type is not audio"<<std::endl;
			reply->result = -1;
        }
    }
    return len;
}

std::string GetToken(std::string id, std::string secret)
{
	std::ostringstream oss;
	oss<<"grant_type=client_credentials"<<"&"<<"client_id="<<id<<"&"
	   <<"client_secret="<<secret;
	std::string url = "https://openapi.data-baker.com/oauth/2.0/token?"+oss.str();
	std::string token;
	HttpReply reply{TYPE_GET_TOKEN, "", NULL, -1};
	CURL * curl = curl_easy_init();
    struct curl_slist* headers = NULL;
	headers=curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);//设置超时时间
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&reply);
	CURLcode res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		std::cout<<"curl perform fail"<<std::endl;
	}
	else
	{
		Json::Reader reader;
		Json::Value root;
		if(reader.parse(reply.response, root))
		{
			if(root.isMember("access_token"))
			{
				token = root["access_token"].asString();
			}
			else
			{
				std::cout<<"cannot find token"<<std::endl;
			}		
		}
		else
		{
			std::cout<<"response format error"<<std::endl;
		}
	}
	curl_easy_cleanup(curl);	
	return token;	
}

int SendGetRequest(RequestParam param)
{
	int ret = -1;		
	std::string token = GetToken(param.client_id, param.client_secret);
	if(token.empty())
	{
		std::cout<<"get token fail"<<std::endl;
		return -1;
	}
	std::ostringstream oss;
	oss<<"text="<<param.text<<"&access_token="<<token<<"&domain="<<param.domain<<"&language="
	<<param.language<<"&speed="<<param.speed<<"&volume="<<param.volume<<"&pitch="<<param.pitch<<
	"&audiotype="<<param.audiotype<<"&rate="<<param.rate<<"&voice_name="<<param.voice_name<<"&spectrum="
	<<param.spectrum<<"&specturm_8k="<<param.spectrum_8k<<"&interval="<<param.interval;
	std::string url = "https://openapi.data-baker.com/tts?"+oss.str();
	HttpReply reply{TYPE_GET_AUDIO, "", fopen("restful.mp3", "wb"), -1};
	CURL * curl = curl_easy_init();
    struct curl_slist* headers = NULL;
	headers=curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);//设置超时时间
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&reply);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&reply);
	CURLcode res = curl_easy_perform(curl);
	ret = reply.result;
	if(res != CURLE_OK)
	{
		std::cout<<"curl perform fail"<<std::endl;
		ret = -1;
	}
	else
	{
		if(ret != 0)
		{
			std::cout<<"Request Fail : "<<reply.response<<std::endl;
		}
		else
		{
			std::cout<<"Request success get audio file : restful.mp3"<<std::endl;
		}
	}
	if(reply.fp)
	{
		fclose(reply.fp);
	}
	curl_easy_cleanup(curl);	
	return ret;
}

int main()
{
	RequestParam param;
	param.client_id 		= "****";//使用自己的client_id替换掉****
   param.client_secret	= "****";//使用自己的client_secret替换掉****
   param.domain			= 1;
	param.text 				= "欢迎使用标贝科技公司语音合成服务";
  	param.voice_name 		= "Jiaojiao";
   param.language 		= "zh";
   param.speed 			= 5;
   param.volume 			= 5;
   param.pitch 			= 5;
   param.audiotype 		= 3;
   param.rate 				= 2;
   param.spectrum 		= 1;
   param.spectrum_8k 	= 0;
   param.interval 		= 1;	
	SendGetRequest(param);
	return 0;	
}
