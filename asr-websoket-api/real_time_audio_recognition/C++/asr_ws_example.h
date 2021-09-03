#ifndef __WSTTS_EXAMPLE_H__
#define __WSTTS_EXAMPLE_H__


#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <list>
#include <boost/thread.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef client::connection_ptr connection_ptr;


struct asr_params
{
    /*必填  原始音频每包固定5120字节，最后一包除外, 且需使用base64进行编码*/
    std::string audio_data;
    /*必填 
    * 音频编码格式
        pcm
        wav
    */
    std::string audio_format;
    /* 必填
    * 音频采样率
        8000
        16000
    */
    int sample_rate;
    /* 必填
    * 音频序号索引，步长为1递增 0：起始音频帧 >0：中间音频帧（如1 2 3 4 … 1000） -n：结束音频帧（如-1001)
    */
    int req_idx;
    /* 非必填
    * 模型名称，通用模型 "common"，
        英文模型"english"，
        默认值为“common”
    */
    std::string domain;
    /* 非必填
    * true: 加标点，默认值 false：不添加标点
    */
    bool add_pct;
    asr_params()
    {
        domain = "common";
        add_pct = true;
    }
};

class asr_ws_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::string gen_json_request(const std::string& token, const std::string& version, const asr_params& params);
public:
    asr_ws_example(const std::string& filename, const std::string& access_token, const std::string& version, const asr_params& params);
    virtual ~asr_ws_example();
    bool open_connection(const std::string& uri);
    void send_request_frame(websocketpp::connection_hdl hdl, const std::string& data);
    void stop_io_service();
    void close_connection();
protected:
    context_ptr on_tls_init(websocketpp::connection_hdl);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
private:
    void work_thread();
#ifdef __ASR_REALTIME_ENABLE__
    void recv_asr_realtime_msg(const std::string& msg);
#else
    void recv_asr_msg(const std::string& msg);
#endif // __ASR_REALTIME_ENABLE__
private:
    client                              ws_client_;
    websocketpp::connection_hdl         hdl_;
    boost::mutex                        hdl_mutex_;
    boost::thread*                      work_thread_;
    std::string                         input_filename_;
    std::string                         access_token_;
    std::string                         version_;
    asr_params                          asr_params_;
};

#endif // __WSTTS_EXAMPLE_H__
