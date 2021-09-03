#ifndef __ASR_RESTFULAPI_EXAMPLE_H__
#define __ASR_RESTFULAPI_EXAMPLE_H__

#include <string>
#include <list>

struct asr_params
{
    /* 必填
    * 通过client_id，client_secret调用授权服务获得见获取访问令牌
    */
    std::string  access_token;
    /* 必填
    * 音频编码格式
      wav
      pcm
    */
    std::string audio_format;
    /* 必填
    * 音频采样率
        8000
        16000
    */
    int sample_rate;
    /* 非必填
    * 模型名称，通用模型 "common"，
    英文模型"english"，
    默认值为“common”
    */
    std::string domain;
    /* 非必填
    * true: 加标点，默认值
      false：不添加标点
    */
    std::string add_pct;
    asr_params()
    {
        domain = "common";
        add_pct = "true";
    }
};

class asr_restfulapi_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::list<std::string> gen_request_header(const asr_params& req_params);
    static bool read_file(const std::string& filename, std::string& data);
public:
    asr_restfulapi_example(const std::string& url);
    ~asr_restfulapi_example();
    bool send_request(const std::string& filename, const asr_params& req_params);
protected:
    std::string url_;
};

#endif // __ASR_RESTFULAPI_EXAMPLE_H__
