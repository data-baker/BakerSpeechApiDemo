#include "tts_longtext_example.h"
#include "httpclient.h"
#include <json/json.h>
#include <iostream>
#include <list>

using std::string;
using std::list;

string tts_longtext_example::get_token(const string& client_id, const string& client_secret)
{
    string url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=";
    url += "client_credentials";
    url += "&client_secret=" + client_secret;
    url += "&client_id=" + client_id;
    http_client client;
    string response;
    if (!client.get(url, response)) {
        return "";
    }
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(response, root)) {
        std::cout << "response format error" << std::endl;
        return "";
    }
    string token;
    if (root.isMember("access_token")) {
        token = root["access_token"].asString();
    }
    return token;
}

std::string tts_longtext_example::gen_json_request(const request_params& req_params)
{
    Json::FastWriter writer;
    Json::Value root;
    root["access_token"] = req_params.access_token;
    root["text"] = req_params.text;
    root["voiceName"] = req_params.voice_name;
    root["notifyUrl"] = req_params.notify_url;
    return writer.write(root);
}

tts_longtext_example::tts_longtext_example(const string& long_text_url)
{
    long_text_url_ = long_text_url;
}

tts_longtext_example::~tts_longtext_example()
{
}

bool tts_longtext_example::send_request(const request_params& req_params)
{
    if (req_params.access_token.empty() || req_params.text.empty() || 
        req_params.voice_name.empty() || req_params.notify_url.empty()) {
        std::cout << "request parameter null " << std::endl;
        return false;
    }
    string json_data = gen_json_request(req_params);
    string response;
    http_client client;
    list<string> header_list;
    header_list.push_back("Content-Type:application/json;charset=UTF-8");
    if (!client.post(long_text_url_, json_data, response, header_list)) {
        std::cout << "post url: " << long_text_url_ << " fail " << std::endl;
        return false;
    }
    // 打印返回信息
    std::cout << "response: " << response << std::endl;
    return false;
}
