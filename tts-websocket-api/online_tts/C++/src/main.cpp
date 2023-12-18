#include "httpclient.h"
#include "tts_ws_example.h"
#include <string>
#include <list>

using namespace std;


void usage()
{
    std::cout << "### this is the websocket api demo for tts" << std::endl;
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
    string ws_url = "wss://openapi.data-baker.com/tts/wsapi";

    // 文本utf8编码，长度不能超过300汉字
    string text = "欢迎使用标贝科技公司语音合成服务";
    string version = "2.1";

    tts_params params;
    params.voice_name = "Jingjing";
    params.text = text;
    params.timestamp = "both";
    params.sample_rate = 16000;
    params.audio_fmt = "PCM";
    // 准备好请求参数
    string json_data = tts_ws_example::build_json(access_token, version, params);

    // 启动websocket client
    tts_ws_example ws_client(json_data);
    if (!ws_client.open_connection(ws_url)) {
        std::cout << "open websocket connection failed !!! websocket url: " << ws_url << std::endl;
    }
    return 0;
}