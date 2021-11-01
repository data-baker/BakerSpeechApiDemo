#include "httpclient.h"
#include "tts_ws_example.h"
#include <string>
#include <list>

using namespace std;

string prepare_req_params(const std::string& access_token, const std::string& text, const std::string& version)
{
    tts_params params;
    params.text = text;
    params.language = "ZH";
    params.voice_name = "Lingling";
    params.interval = "1";
    return tts_ws_example::gen_json_request(access_token, version, params);;
}

void usage()
{
    std::cout << "### this is a websocket api demo for tts" << std::endl;
    std::cout << "###  tts_ws_example [client_id] [client_secret]" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "miss run parameter !!!" << std::endl;
        usage();
        return -1;
    }
    // 填写自己的  client_id  client_secret
    string client_id = argv[1];
    string client_secret = argv[2];

    // 获取access token
    string access_token = tts_ws_example::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // websocket 地址
    string ws_url = "wss://openapi.data-baker.com/wss";

    // 文本长度不能超过1024字节
    string text = "今天天气不错哦";
    string version = "1.0";
    // 准备好请求参数
    string json_data = prepare_req_params(access_token, text, version);

    // 启动websocket client
    tts_ws_example ws_client(json_data);
    if (!ws_client.open_connection(ws_url)) {
        std::cout << "open websocket connection failed !!! websocket url: " << ws_url << std::endl;
    }
    return 0;
}