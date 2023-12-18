#ifndef __TTS_RESTFULAPI_DEMO__H__
#define __TTS_RESTFULAPI_DEMO__H__

#include <string>

struct req_tts_params
{
    std::string voice_name;
    std::string text;
    std::string audio_fmt;
    std::string timestamp;
    int sample_rate;
    req_tts_params()
    {
        sample_rate = 16000;
        audio_fmt = "WAV";
        voice_name = "Jingjing";
    }
};

class tts_restfulapi_demo
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static void write_file(const std::string& filiename, const char* data, int size);
public:
    tts_restfulapi_demo(const std::string req_url): req_url_(req_url) {}
    ~tts_restfulapi_demo() {}
    std::string build_json(const std::string& access_token, const req_tts_params& tts_params);
    bool send_request(const std::string& access_token, const req_tts_params& req_params);
protected:
private:
    std::string req_url_;
};

#endif// __TTS_RESTFULAPI_DEMO__H__
