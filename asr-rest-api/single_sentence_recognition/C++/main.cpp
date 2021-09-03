

#include "httpclient.h"
#include "asr_restfulapi_example.h"
#include <string>
#include <iostream>

using namespace std;

void usage()
{
    std::cout << "### this is a restful api demo for asr" << std::endl;
    std::cout << "###  asr_restfulapi_example [client_id] [client_secret]" << std::endl;
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
    string access_token = asr_restfulapi_example::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // api url
    string url = "https://openapi.data-baker.com/asr/api?";

    // 准备好请求参数, 根据自己语音情况填写
    asr_params params;
    params.access_token = access_token;
    params.audio_format = "pcm";
    params.sample_rate = 16000;

    string filename = "abc.pcm";

    asr_restfulapi_example example(url);
    example.send_request(filename, params);

    return 0;
}