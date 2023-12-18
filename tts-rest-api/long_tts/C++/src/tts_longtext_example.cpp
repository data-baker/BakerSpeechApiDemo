#include "tts_longtext_example.h"
#include "httpclient.h"
#include <json/json.h>
#include <iostream>
#include <list>
#include <chrono>
#include <iostream>
#include <thread>

using std::string;
using std::list;

string tts_longtext_example::get_token(const string& client_id, const string& client_secret)
{
    string url = "http://10.10.50.23:9904/oauth/2.0/token?grant_type=";
    // string url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=";
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

std::string tts_longtext_example::gen_json_request(const std::string& access_token, const request_params& req_params)
{
    Json::Value root;
    root["version"] = "2.1";
    root["access_token"] = access_token;
    Json::Value tts_params_node;
    tts_params_node["language"] = "ZH";
    tts_params_node["text"] = req_params.text;
    tts_params_node["voice_name"] = req_params.voice_name;
    tts_params_node["audio_fmt"] = req_params.audio_fmt;
    tts_params_node["sample_rate"] = req_params.sample_rate;
    tts_params_node["timestamp"] = req_params.timestamp;
    tts_params_node["notify_url"] = req_params.notify_url;
    root["tts_params"] = tts_params_node;
    Json::FastWriter writer;
    return writer.write(root);
}

tts_longtext_example::tts_longtext_example(const string& long_text_url, const std::string& long_text_query_url)
{
    long_text_url_ = long_text_url;
    long_text_query_url_ = long_text_query_url;
}

tts_longtext_example::~tts_longtext_example()
{
}

bool tts_longtext_example::send_request(const std::string& access_token, const std::string& client_id, const request_params& req_params)
{
    if (access_token.empty() || req_params.text.empty() || 
        req_params.voice_name.empty()) {
        std::cout << "request parameter null " << std::endl;
        return false;
    }
    string json_data = gen_json_request(access_token, req_params);
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
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response, root)) {
        return false;
    }
    if (!root.isMember("err_no")) {
        return false;
    }
    int err_no = root["err_no"].asInt();
    if (err_no != 0) {
        return false;
    }
    if (!root.isMember("result")) {
        return false;
    }
    string work_id = root["result"]["work_id"].asString();
    string query_url = long_text_query_url_;
    query_url += "?client_id=";
    query_url += client_id;
    query_url += "&access_token=";
    query_url += access_token;
    query_url += "&work_id=";
    query_url += work_id;
    query_url += "&version=2.1";
    while (true) {
        std::cout << "query result" << std::endl;
        response = "";
        if (!client.get(query_url, response)) {
            std::cout << "send query http request failed" << std::endl;
            return false;
        }
        std::cout << "query response: " << response << std::endl;
        root = Json::Value();
        if (!reader.parse(response, root)) {
            std::cout << "query response json parse failed" << std::endl;
            return false;
        }
        if (!root.isMember("err_no")) {
            return false;
        }
        err_no = root["err_no"].asInt();
        if (0 == err_no) {
            std::cout << "query success" << std::endl;
            break;
        }
        if (21040001 != err_no) {
            std::cout << "work failed" << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return true;
}
