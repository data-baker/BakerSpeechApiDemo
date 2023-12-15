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
    std::string voice_name;
    std::string text;
    std::string audio_fmt;
    std::string timestamp;
    int sample_rate;
    tts_params()
    {
        sample_rate = 16000;
        audio_fmt = "PCM";
        voice_name = "Jingjing";
    }
};


class tts_ws_example
{
public:
    static std::string get_token(const std::string& client_id, const std::string& client_secret);
    static std::string build_json(const std::string& token, const std::string& version, const tts_params& params);
public:
    tts_ws_example(const std::string& json_data);
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
    std::string                         json_data_;
    FILE*                               output_file_;
    std::string                         output_filename_;
};

#endif // __WSTTS_EXAMPLE_H__
