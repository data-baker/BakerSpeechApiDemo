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


struct tts_params
{
    /*必填  合成的文本，使用UTF-8编码，长度不超过1024字节，且需进行base64编码*/
    std::string text;
    /*必填 应用所属领域如导航、客服等，以数字进行编码，目前值固定为1*/
    std::string domain;
    /* 必填
    * 合成请求文本的语言
      ZH(中文和中英混)
      ENG(纯英文，中文部分不会合成)
      CAT(粤语）
      SCH(四川话)
    */
    std::string language;
    /* 非必填
    * 设置播放的语速，在0～9之间（支持浮点值），默认值为5
    */
    std::string speed;
    /* 非必填
    * 设置语音的音量，在0～9之间（只支持整型值），默认值为5
    */
    std::string volume;
    /* 非必填
    * 设置语音的音调，在0～9之间，（支持浮点值），默认值为5
    */
    std::string pitch;
    /* 非必填
    * 音频种类：
        audiotype = 4，返回16K采样率的pcm格式，默认值
        audiotype = 5，返回8K采样率的pcm格式
    */
    std::string audiotype;
    /* 必填
    * 发音人选择，如“Jiaojiao", 详见 发音人列表
    */
    std::string voice_name;
    /* 非必填
    * 高频频谱：取值范围1~20； 默认值为1，不调整频谱；
        1代表不调整频谱；
        1以上的值代表高频能量增加幅度， 值越大声音的高频部分增强越多，听起来更亮和尖细
    */
    std::string spectrum;
    /* 非必填
    * 低频部分频谱：取值范围0~20；默认值为0，仅针对8K音频频谱的调整。 组合形式只有以下几种：
        audiotype=5&spectrum_8k=xx
        audiotype=6&rate=1&spectrum_8k=xx
        audiotype=7&spectrum_8k=xx
        audiotype=8&spectrum_8k=xx
    */
    std::string spectrum_8k;
    /* 非必填
    * 取值0/1，interval=1时返回音子时间戳信息
    */
    std::string interval;
    tts_params()
    {
        domain = "1";
        speed = "5";
        volume = "5";
        pitch = "5";
        audiotype = "4";
        spectrum = "1";
        spectrum_8k = "0";
    }
};

class tts_ws_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::string gen_json_request(const std::string& token, const std::string& version, const tts_params& params);
public:
    tts_ws_example(const std::list<std::string>& json_data_list);
    virtual ~tts_ws_example();
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
    void work_thread();
    void recv_audio_frame(const std::string& msg);
    void close_output_file();
private:
    client                              ws_client_;
    websocketpp::connection_hdl         hdl_;
    boost::mutex                        hdl_mutex_;
    boost::thread*                      work_thread_;
    std::list<std::string>              json_data_list_;
    FILE*                               output_file_;
    std::string                         output_filename_;
};

#endif // __WSTTS_EXAMPLE_H__
