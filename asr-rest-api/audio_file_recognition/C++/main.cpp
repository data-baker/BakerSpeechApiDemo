
#include "httpclient.h"
#include "asr_filerestapi_example.h"
#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;

void usage()
{
    std::cout << "### this is a file restful api demo for asr" << std::endl;
    std::cout << "###  asr_filerestapi_example [client_id] [client_secret]" << std::endl;
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
    string access_token = asr_filerestapi_example::get_token(client_id, client_secret);
    if (access_token.empty()) {
        std::cout << "get access token failed !!!" << std::endl;
        return -1;
    }
    // api url
    string url = "https://openapi.data-baker.com/asr/starttask?";

    // 回调通知url, 为空时可以主动轮询 查询结果
    string callback_url = "";

    // 根据自己音频格式设置，支持三种音频格式：pcm、wav、mp3
    string audio_fmt = "pcm";

    // 根据自己音频设置音频采样率
    int sample_rate = 16000;

    ////支持两种上传音频方式：url或文件流
    ////1.file url方式
    //// 要识别文件的url 地址
    //string file_url = "";
    //asr_task_type_t task_type = att_FileUrl;

    //2.文件流方式
    // 要识别的文件名字
    string file_url = "abc.pcm";
    asr_task_type_t task_type = att_AudioData;

    asr_filerestapi_example example(url, access_token);
    string taskid;
    if (!example.start_task(task_type, file_url, callback_url, audio_fmt, sample_rate, taskid)) {
        std::cout << "start asr task failed !!!" << std::endl;
        return -1;
    }

    // 暂停一段时间 在进行查询
    std::cout << "sleep 60s " << std::endl;
    sleep(60);

    // 回调url 为空，taskid 不为空时，开始查询结果，可以根据自己需要开启线程定时去查询
    if (callback_url.empty() && taskid.size() > 0) {
        string query_url = "https://openapi.data-baker.com/asr/taskresult?";
        example.query_task_result(query_url, taskid);
    }
    return 0;
}