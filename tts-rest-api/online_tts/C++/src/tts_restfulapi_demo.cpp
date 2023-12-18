#include "tts_restfulapi_demo.h"
#include "httpclient.h"
#include "json/json.h"

using namespace std;

std::string tts_restfulapi_demo::get_token(const std::string& client_id, const std::string& client_secret)
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

void tts_restfulapi_demo::write_file(const std::string& filiename, const char* data, int size)
{
    FILE* fp = fopen(filiename.c_str(), "wb");
    fwrite(data, 1, size, fp);
    fclose(fp);
    fp = NULL;
}

string tts_restfulapi_demo::build_json(const std::string& access_token, const req_tts_params& tts_params)
{
    Json::Value root;
    root["version"] = "2.1";
    root["access_token"] = access_token;
    Json::Value tts_params_node;
    tts_params_node["domain"] = 1;
    tts_params_node["language"] = "ZH";
    tts_params_node["text"] = tts_params.text;
    tts_params_node["voice_name"] = tts_params.voice_name;
    tts_params_node["audio_fmt"] = tts_params.audio_fmt;
    tts_params_node["sample_rate"] = tts_params.sample_rate;
    tts_params_node["timestamp"] = tts_params.timestamp;
    root["tts_params"] = tts_params_node;
    Json::FastWriter writer;
    return writer.write(root);
}

bool tts_restfulapi_demo::send_request(const std::string& access_token, const req_tts_params& tts_params)
{
    string response, content_type;
    string request_str = build_json(access_token, tts_params);
    http_client client;
    if (!client.post(req_url_, request_str, response, content_type)) {
        std::cout << "send http request failed. " << std::endl;
        return -1;
    }
    if (string::npos != content_type.find("json")) {
        std::cout << "request tts failed. response: " << response << std::endl;
        return -1;
    }
    string filename = tts_params.voice_name;
    filename += ".";
    filename += content_type.substr(sizeof("audio/") - 1);
    if (tts_params.timestamp.empty()) {
        write_file(filename, response.c_str(), response.size());
        std::cout << "output: " << filename << std::endl;
        return 0;
    }
    // 开始处理response
    if (response.size() < 5) { // 判断返回的response长度
        std::cout << "response size wrong size:" << response.size() << std::endl;
        return -1;
    }
    const unsigned char* data = (const unsigned char*)response.data();
    int json_size = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
    if ((json_size + 4) > response.size()) {
        std::cout << "response size wrong size:" << response.size() << std::endl;
        return -1;
    }
    // json 数据
    string json_data((const char*)data + 4, json_size);
    std::cout << "timestamp: " << json_data << std::endl;
    // 音频数据
    string audio((const char*)data + 4 + json_size, response.size() - 4 - json_size);
    write_file(filename, audio.c_str(), audio.size());
    std::cout << "output: " << filename << std::endl;
 }
