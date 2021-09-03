#include "httpclient.h"
#include "asr_ws_example.h"
#include <string>
#include <iostream>

using namespace std;

void usage()
{
    std::cout << "### this is a websocket api demo for asr" << std::endl;
    std::cout << "###  asr_ws_example [client_id] [client_secret]" << std::endl;
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
    string access_token = asr_ws_example::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // websocket 地址, 一句话识别url 或者 实时长语音识别 url
#ifdef __ASR_REALTIME_ENABLE__
    string ws_url = "wss://openapi.data-baker.com/asr/realtime";
#else
    string ws_url = "wss://openapi.data-baker.com/asr/wsapi";
#endif

    string version = "1.0";
    // 如果是实时长语音, 可以不用语音文件，可以实时把音频数据通过websocket 发送
    string filename = "abc.pcm";

    asr_params params;
    // 根据自己音频格式具体设置
    params.audio_format = "pcm";
    // 根据自己音频设置对应采样率
    params.sample_rate = 16000;

    // 启动websocket client
    asr_ws_example ws_client(filename, access_token, version, params);
    if (!ws_client.open_connection(ws_url)) {
        std::cout << "open websocket connection failed !!! websocket url: " << ws_url << std::endl;
    }
    return 0;
}