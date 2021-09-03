#include "asr_restfulapi_example.h"
#include "httpclient.h"
#include <json/json.h>
#include <iostream>

using std::string;
using std::list;

string asr_restfulapi_example::get_token(const string& client_id, const string& client_secret)
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

list<std::string> asr_restfulapi_example::gen_request_header(const asr_params& req_params)
{
    list<string> result;
    string tmp;
    tmp = "access_token:" + req_params.access_token;
    result.push_back(tmp);
    tmp = "audio_format:" + req_params.audio_format;
    result.push_back(tmp);
    tmp = "sample_rate:" + std::to_string(req_params.sample_rate);
    result.push_back(tmp);
    if (req_params.domain.size() > 0) {
        tmp = "domain:" + req_params.domain;
        result.push_back(tmp);
    }
    if (req_params.add_pct.size() > 0) {
        tmp = "add_pct:" + req_params.add_pct;
        result.push_back(tmp);
    }
    
    return result;
}

bool asr_restfulapi_example::read_file(const std::string& filename, std::string& data)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        return false;
    }
    char buffer[1024];
    int size = 0;
    while (!feof(fp)) {
        size = fread(buffer, 1, sizeof(buffer), fp);
        if (size <= 0) {
            break;
        }
        data.append(buffer, size);
    }
    fclose(fp);
    return true;
}

asr_restfulapi_example::asr_restfulapi_example(const std::string& url)
{
    url_ = url;
}

asr_restfulapi_example::~asr_restfulapi_example()
{
}

bool asr_restfulapi_example::send_request(const string& filename, const asr_params& req_params)
{
    if (filename.empty() || req_params.access_token.empty() || req_params.audio_format.empty()) {
        std::cout << "request parameter null " << std::endl;
        return false;
    }
    // 读取出音频文件内容
    string data;
    if (!read_file(filename, data)) {
        std::cout << "read file failed . " << std::endl;
        return false;
    }
    std::cout << "audio file length: " << data.length() << std::endl;
    // 拼接请求头
    list<string> req_headers = gen_request_header(req_params);
    string response;
    http_client client;
    if (!client.post(url_, data, response, req_headers)) {
        std::cout << "post url: " << url_ << " fail " << std::endl;
        return false;
    }
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(response, root)) {
        std::cout << "json reader response failed !!!" << std::endl;
        return false;
    }
    if (root["code"].asInt() != 20000) {
        std::cout << "call restful api failed... reseponse: " << response << std::endl;
        return false;
    }
    string text = root["text"].asString();
    std::cout << "### Success. text: " << text << std::endl;
    return true;
}
