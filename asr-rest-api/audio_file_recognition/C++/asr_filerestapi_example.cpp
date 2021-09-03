#include "asr_filerestapi_example.h"
#include "httpclient.h"
#include <json/json.h>
#include <iostream>

using std::string;
using std::list;

string asr_filerestapi_example::get_token(const string& client_id, const string& client_secret)
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

list<string> asr_filerestapi_example::gen_request_header(const string& access_token, const string& file_url, const string& callback_url, const string& audio_format, int sample_rate)
{
    list<string> result;
    string tmp;
    tmp = "access_token:" + access_token;
    result.push_back(tmp);
    if (file_url.size() > 0) {
        tmp = "file_url:" + file_url;
        result.push_back(tmp);
    }
    if (callback_url.size() > 0) {
        tmp = "callback_url:" + callback_url;
        result.push_back(tmp);
    }
    tmp = "audio_format:" + audio_format;
    result.push_back(tmp);
    tmp = "sample_rate:" + std::to_string(sample_rate);
    result.push_back(tmp);

    return result;
}

bool asr_filerestapi_example::read_file(const string& filename, string& data)
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

asr_filerestapi_example::asr_filerestapi_example(const string& url, const string& access_token)
{
    url_ = url;
    access_token_ = access_token;
}

asr_filerestapi_example::~asr_filerestapi_example()
{
}

bool asr_filerestapi_example::start_task(asr_task_type_t task_type, const string& file_url, const string& callback_url, const string& audio_format, int sample_rate, string& taskid)
{
    if (file_url.empty() || audio_format.empty()) {
        std::cout << "request parameter null " << std::endl;
        return false;
    }
    if (sample_rate != 8000 && sample_rate != 16000) {
        std::cout << "request sample rate error " << std::endl;
        return false;
    }
    list<string> req_headers;
    // 保存音频文件内容
    string data;
    if (att_FileUrl == task_type) {
        // 拼接请求头
        req_headers = gen_request_header(access_token_, file_url, callback_url, audio_format, sample_rate);
    } else {
        // 读取出音频文件内容
        if (!read_file(file_url, data)) {
            std::cout << "read file failed . " << std::endl;
            return false;
        }
        std::cout << "audio file length: " << data.length() << std::endl;
        // 拼接请求头
        req_headers = gen_request_header(access_token_, "", callback_url, audio_format, sample_rate);
    }
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
    taskid = root["taskid"].asString();
    std::cout << "### Success. taskid: " << taskid << std::endl;
    return true;
}

bool asr_filerestapi_example::query_task_result(const string& query_url, const std::string& taskid)
{
    list<string> header_list;
    string tmp;
    tmp = "access_token:" + access_token_;
    header_list.push_back(tmp);
    tmp = "taskid:" + taskid;
    header_list.push_back(tmp);
    string response;
    http_client client;
    if (!client.post(query_url, "", response, header_list)) {
        std::cout << "call asr file RESTful query result failed !!!" << std::endl;
        return false;
    }
    std::cout << "Task result: " << response << std::endl;
    // 需要自己使用处理json 结果
    return true;
}
