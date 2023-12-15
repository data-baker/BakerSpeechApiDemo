#include <string>
#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "tts_restfulapi_demo.h"


using namespace std;

void usage()
{
    std::cout << "### this is the restful api demo for tts" << std::endl;
    std::cout << "###  tts_restful_demo [client_id] [client_secret]" << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "miss run parameter !!!" << std::endl;
        usage();
        return -1;
    }
    string voice_name = "Jingjing";

    // 填写自己的  client_id  client_secret
    string client_id = argv[1];
    string client_secret = argv[2];

    // 获取access token
    string access_token = tts_restfulapi_demo::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // 请求 地址
    string req_url = "https://openapi.data-baker.com/tts";

    string text = "欢迎使用标贝科技公司语音合成服务";
    req_tts_params tts_params;
    tts_params.voice_name = voice_name;
    tts_params.text = text;
    tts_params.timestamp = "both";
    tts_params.sample_rate = 16000;
    tts_params.audio_fmt = "WAV";

    tts_restfulapi_demo tts(req_url);
    tts.send_request(access_token, tts_params);
    return 0;
}
