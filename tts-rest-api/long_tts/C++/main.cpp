#include "tts_longtext_example.h"
#include <string>
#include <iostream>

using namespace std;

void usage()
{
    std::cout << "### this is a longtext api demo for tts" << std::endl;
    std::cout << "###  tts_longtext_example [client_id] [client_secret]" << std::endl;
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
    string access_token = tts_longtext_example::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // 长文本api url
    string long_text_url = "https://openapi.data-baker.com/asynctts/synthesis/work";
    // 长文本轮询结果
    string long_text_query_url = "https://openapi.data-baker.com/asynctts/synthesis/query";

    // 准备好请求参数
    request_params params;
    // 填写自己的具体长文本
    params.text = "欢迎使用标贝科技公司语音合成服务";
    params.voice_name = "Jingjing";
    params.sample_rate = 16000;
    params.audio_fmt = "WAV";
    params.timestamp = 0;
    // 填写自己的接收通知地址
    // params.notify_url = "";

    tts_longtext_example example(long_text_url, long_text_query_url);
    example.send_request(access_token, client_id, params);

    return 0;
}