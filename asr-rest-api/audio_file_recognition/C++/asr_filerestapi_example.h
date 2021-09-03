#ifndef __ASR_FILERESTFULAPI_EXAMPLE_H__
#define __ASR_FILERESTFULAPI_EXAMPLE_H__

#include <string>
#include <list>

enum asr_task_type_t
{
    att_FileUrl = 0,
    att_AudioData,
};

class asr_filerestapi_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::list<std::string> gen_request_header(const std::string& access_token, const std::string& file_url, const std::string& callback_url, const std::string& audio_format, int sample_rate);
    static bool read_file(const std::string& filename, std::string& data);
public:
    asr_filerestapi_example(const std::string& url, const std::string& access_token);
    ~asr_filerestapi_example();
    bool start_task(asr_task_type_t task_type, const std::string& file_url, const std::string& callback_url, const std::string& audio_format, int sample_rate, std::string& taskid);
    bool query_task_result(const std::string& query_url, const std::string& taskid);
protected:
    std::string url_;
    std::string access_token_;
};

#endif // __ASR_FILERESTFULAPI_EXAMPLE_H__
