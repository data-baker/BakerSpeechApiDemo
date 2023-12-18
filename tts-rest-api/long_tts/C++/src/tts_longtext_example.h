#ifndef __TTS_LONGTEXT_EXAMPLE_H__
#define __TTS_LONGTEXT_EXAMPLE_H__

#include <string>


struct request_params
{
    std::string voice_name;
    std::string text;
    std::string audio_fmt;
    int timestamp;
    int sample_rate;
    std::string notify_url;
    request_params()
    {
        sample_rate = 16000;
        audio_fmt = "PCM";
        voice_name = "Jingjing";
        timestamp = 0;
    }
};


class tts_longtext_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::string gen_json_request(const std::string& access_token, const request_params& req_params);
public:
    tts_longtext_example(const std::string& long_text_url, const std::string& long_text_query_url);
    ~tts_longtext_example();
    bool send_request(const std::string& access_token, const std::string& client_id, const request_params& req_params);
protected:
    std::string long_text_url_;
    std::string long_text_query_url_;
};

#endif // __TTS_LONGTEXT_EXAMPLE_H__
