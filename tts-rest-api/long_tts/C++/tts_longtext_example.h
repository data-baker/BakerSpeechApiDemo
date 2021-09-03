#ifndef __TTS_LONGTEXT_EXAMPLE_H__
#define __TTS_LONGTEXT_EXAMPLE_H__

#include <string>

struct request_params
{
    /* 必填
    * 通过client_id，client_secret调用授权服务获得见获取访问令牌
    */
    std::string  access_token;
    /* 必填
    * 需要合成的文本（注意换行符需url编码）支持一次性上传不超过10万个字节
    */
    std::string  text;
    /* 必填
    * 设置发音人声音名称，详见 发音人列表
    */
    std::string  voice_name;
    /* 必填
    * 该地址用于接收合成结果，该地址必须为外网可访问的url，不能携带参数
    */
    std::string  notify_url;
    /* 非必填
    * 是否携带interval信息：
        0：不携带，默认值
        1：携带
    */
    int          interval;
    /* 非必填
    * 设置播放的语速，在0～9之间（支持一位浮点值），不传时默认为5
    */
    std::string  speed;
    /* 非必填
    * 设置语音的音量，在0～9之间（只支持整型值），不传时 默认值为5
    */
    std::string  volume;
    /* 非必填
    * 可不填
    audiotype=3 ：返回mp3格式
    audiotype=4 ：返回16K采样率的pcm格式
    audiotype=6 ：返回16K采样率的wav格式
    */
    int          audiotype;
    request_params()
    {
        interval = 0;
        speed = "5";
        volume = "5";
        audiotype = 0;
    }
};

class tts_longtext_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::string gen_json_request(const request_params& req_params);
public:
    tts_longtext_example(const std::string& long_text_url);
    ~tts_longtext_example();
    bool send_request(const request_params& req_params);
protected:
    std::string long_text_url_;
};

#endif // __TTS_LONGTEXT_EXAMPLE_H__
