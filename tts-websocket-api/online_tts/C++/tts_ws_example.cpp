#include "tts_ws_example.h"
#include "httpclient.h"
#include <string>
#include <iostream>
#include <json/json.h>

using std::string;
using std::ostringstream;
using std::cout;
using std::endl;

using websocketpp::lib::bind;

tts_ws_example::tts_ws_example(const std::string& json_data)
{
    json_data_ = json_data;
    ws_client_.set_access_channels(websocketpp::log::alevel::all);
    ws_client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    ws_client_.set_error_channels(websocketpp::log::elevel::all);

    ws_client_.init_asio();
    ws_client_.set_open_handshake_timeout(10000);

    ws_client_.set_tls_init_handler(bind(&tts_ws_example::on_tls_init,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_open_handler(bind(&tts_ws_example::on_open,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_close_handler(bind(&tts_ws_example::on_close,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_fail_handler(bind(&tts_ws_example::on_fail,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_message_handler(bind(&tts_ws_example::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    work_thread_ = NULL;
    output_file_ = NULL;
    output_filename_ = "abc.pcm";
}

tts_ws_example::~tts_ws_example()
{
    stop_io_service();
    if (work_thread_ != NULL) {
        work_thread_->join();
        delete work_thread_;
        work_thread_ = NULL;
    }
    close_output_file();
}

string tts_ws_example::get_token(const string& client_id, const string& client_secret)
{
    string url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=";
    url += "client_credentials";
    url += "&client_secret=" + client_secret;
    url += "&client_id=" + client_id;
    http_client client;
    string response;
    if (!client.get(url, response)) {
        return "";
    }
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(response, root)) {
        std::cout << "response format error" << std::endl;
        return "";
    }
    string token;
    if (root.isMember("access_token")) {
        token = root["access_token"].asString();
    }
    return token;
}

string tts_ws_example::gen_json_request(const std::string& token, const std::string& version, const tts_params& params)
{
    Json::Value root;
    Json::Value tts_params;
    Json::FastWriter writer;

    tts_params["text"] = websocketpp::base64_encode((const unsigned char*)params.text.c_str(), params.text.length());
    tts_params["domain"] = params.domain;
    tts_params["language"] = params.language;
    tts_params["voice_name"] = params.voice_name;
    tts_params["speed"] = params.speed;
    tts_params["volume"] = params.volume;
    tts_params["pitch"] = params.pitch;
    tts_params["audiotype"] = params.audiotype;
    tts_params["interval"] = params.interval;
    tts_params["spectrum"] = params.spectrum;
    tts_params["spectrum_8k"] = params.spectrum_8k;

    root["access_token"] = token;
    root["version"]      = version;
    root["tts_params"]   = tts_params;

    return writer.write(root);
}

void tts_ws_example::recv_audio_frame(const std::string& msg)
{
    std::cout << "recv audio frame length: " << msg.length() << " ,msg: " << msg << std::endl;

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(msg, root)) {
        std::cout << "response format error. " << std::endl;
        return ;
    }
    if (!root.isMember("code")) {
        std::cout << "response code is null. " << std::endl;
        return;
    }
    int code = root["code"].asInt();
    // 返回错误
    if (code != 90000) {
        return;
    }
    if (!root.isMember("data")) {
        std::cout << "response data is null. " << std::endl;
        return;
    }
    Json::Value& dataNode = root["data"];
    int end_flag = dataNode["end_flag"].asInt();
    string audio_data = dataNode["audio_data"].asString();
    std::cout << "audio_data size: " << audio_data.size() << std::endl;
    if (audio_data.size() > 0) {
        string audio_data_decode = websocketpp::base64_decode(audio_data);
        std::cout << "audio_data1 size: " << audio_data_decode.size() << std::endl;
        if (output_file_ != NULL) {
            fwrite(audio_data_decode.c_str(), audio_data_decode.length(), 1, output_file_);
            fflush(output_file_);
        }
    }
    // 结束
    if (1 == end_flag) {
        // 关闭输出文件
        close_output_file();
        // 关闭连接
        close_connection();
        stop_io_service();
        return;
    }
}

void tts_ws_example::close_output_file()
{
    if (output_file_ != NULL) {
        fclose(output_file_);
        output_file_ = NULL;
    }
}

void tts_ws_example::send_request_frame(websocketpp::connection_hdl hdl, const std::string& data)
{
    if(data.empty()) {
        return;
    }
    ostringstream oss;
    try 
    {
        websocketpp::lib::error_code ec;
        ws_client_.send(hdl, data, websocketpp::frame::opcode::TEXT, ec);
        if (ec) {
            oss << "Error: send error: " << ec.message();
        } else {
            oss << "Notice: send websocket request success. request content[" << data << "]";
        }
    } catch (websocketpp::exception& e) {
        oss << "Error: catch websocketpp exception:[" << e.what() << "]";
    } catch(...) {
        oss << "Error: catch other exception";
    }
    cout << oss.str() << endl;
}

bool tts_ws_example::open_connection(const std::string& uri)
{
    ostringstream oss;
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client_.get_connection(uri, ec);
    if (ec) {
        oss << "Error: connection initialized failed: " << ec.message();
        cout << oss.str() << endl;
        return false;
    }

    {
        boost::lock_guard<boost::mutex> lock(hdl_mutex_);
        hdl_ = con->get_handle();
        ws_client_.connect(con);
        oss << "Notice: start connecting, hdl:[" << hdl_.lock().get() << "]";
        cout << oss.str() << endl;
    }
    ws_client_.reset();
    ws_client_.run();
    oss.str("");
    oss << "Notice: after run.";
    cout << oss.str() << endl;
    return true;
}

void tts_ws_example::close_connection()
{
    ostringstream oss;
    try
    {
        boost::lock_guard<boost::mutex> lock(hdl_mutex_);
        if(!hdl_.expired())
        {
            client::connection_ptr con = ws_client_.get_con_from_hdl(hdl_);
            if(con->get_state() == websocketpp::session::state::open)
            {
                int close_code = websocketpp::close::status::normal;
                ws_client_.close(hdl_, close_code, "");
                oss << "Notice: close connection.";
            }
        }
    }catch(websocketpp::exception& e)
    {
        oss << "Error: catch websocketpp exception:[" << e.what() << "]";
    }catch(...)
    {
        oss << "Error: catch other exception";
    }
    cout << oss.str() << endl;
}

void tts_ws_example::stop_io_service()
{
    ostringstream oss;
    try
    {
        ws_client_.stop();
        oss << "Notice: stop io_service.";
    }catch(websocketpp::exception& e)
    {
        oss << "Error: catch websocketpp exception:[" << e.what() << "]";
    }catch(...)
    {
        oss << "Error: catch other exception";
    }
    cout << oss.str() << endl;
}

context_ptr tts_ws_example::on_tls_init(websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

void tts_ws_example::on_open(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_open called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
    {
        boost::thread(boost::bind(&tts_ws_example::work_thread, this)).detach();
    }
}

void tts_ws_example::on_close(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_close called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
}

void tts_ws_example::on_fail(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_fail called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
}

void tts_ws_example::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
    ostringstream oss;
    oss << "Notice: on_message called, " << "hdl:[" << hdl.lock().get()
                                         << "], payload_length:[" << msg->get_payload().length()
                                         << "]";
    cout << oss.str() << endl;
    recv_audio_frame(msg->get_payload());
}

void tts_ws_example::work_thread()
{
    output_file_ = fopen(output_filename_.c_str() , "wb");
    std::cout << "Notice: workThread begin send request data " << std::endl;
    send_request_frame(hdl_, json_data_);
    std::cout << "Notice: workThread end send request data finish" << std::endl;
}

