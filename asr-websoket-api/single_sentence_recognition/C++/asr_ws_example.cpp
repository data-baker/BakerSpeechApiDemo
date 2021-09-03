#include "asr_ws_example.h"

#include <string>
#include <iostream>
#include <vector>
#include <json/json.h>
#include "httpclient.h"

using std::string;
using std::list;
using std::vector;
using std::ostringstream;
using std::cout;
using std::endl;

using websocketpp::lib::bind;

asr_ws_example::asr_ws_example(const std::string& filename, const std::string& access_token, const std::string& version, const asr_params& params)
{
    input_filename_ = filename;
    access_token_ = access_token;
    version_ = version;
    asr_params_ = params;
    ws_client_.set_access_channels(websocketpp::log::alevel::all);
    ws_client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    ws_client_.set_error_channels(websocketpp::log::elevel::all);

    ws_client_.init_asio();
    ws_client_.set_open_handshake_timeout(10000);

    ws_client_.set_tls_init_handler(bind(&asr_ws_example::on_tls_init,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_open_handler(bind(&asr_ws_example::on_open,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_close_handler(bind(&asr_ws_example::on_close,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_fail_handler(bind(&asr_ws_example::on_fail,this, websocketpp::lib::placeholders::_1));
    ws_client_.set_message_handler(bind(&asr_ws_example::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    work_thread_ = NULL;
}

asr_ws_example::~asr_ws_example()
{
    stop_io_service();
    if (work_thread_ != NULL) {
        work_thread_->join();
        delete work_thread_;
        work_thread_ = NULL;
    }
}

string asr_ws_example::get_token(const string& client_id, const string& client_secret)
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

string asr_ws_example::gen_json_request(const std::string& token, const std::string& version, const asr_params& params)
{
    Json::Value root;
    Json::Value json_asr_params;
    Json::FastWriter writer;

    json_asr_params["audio_data"] = params.audio_data;
    json_asr_params["audio_format"] = params.audio_format;
    json_asr_params["sample_rate"] = params.sample_rate;
    json_asr_params["req_idx"] = params.req_idx;
    json_asr_params["domain"] = params.domain;
    json_asr_params["add_pct"] = params.add_pct;

    root["access_token"] = token;
    root["version"]      = version;
    root["asr_params"]   = json_asr_params;

    return writer.write(root);
}


#ifdef __ASR_REALTIME_ENABLE__

void asr_ws_example::recv_asr_realtime_msg(const std::string& msg)
{
    std::cout << "recv text message. length: " << msg.length() << " ,msg: " << msg << std::endl;
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(msg, root)) {
        std::cout << "response format error. " << std::endl;
        return;
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
    int end_flag = root["end_flag"].asInt();
    if (end_flag == 1) {
        // 关闭连接
        close_connection();
        stop_io_service();
        return;
    }
}

#else 

void asr_ws_example::recv_asr_msg(const std::string& msg)
{
    std::cout << "recv text message. length: " << msg.length() << " ,msg: " << msg << std::endl;

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(msg, root)) {
        std::cout << "response format error. " << std::endl;
        return;
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
    Json::Value& dataNode = root["data"];
    int end_flag = dataNode["end_flag"].asInt();
    int res_idx = dataNode["res_idx"].asInt();
    if (dataNode.isMember("nbest")) {
        Json::Value& nbestNode = dataNode["nbest"];
        int nbest_size = nbestNode.size();

        for (int i = 0; i < nbest_size; ++i) {
            std::cout << "### Notice >>> res_idx: " << res_idx << ",asr result: " << nbestNode[i].asString() << std::endl;
        }
    }
    // 结束
    if (1 == end_flag) {
        // 关闭连接
        close_connection();
        stop_io_service();
        return;
    }
}

#endif // __ASR_REALTIME_ENABLE__

void asr_ws_example::send_request_frame(websocketpp::connection_hdl hdl, const std::string& data)
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

bool asr_ws_example::open_connection(const std::string& uri)
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

void asr_ws_example::close_connection()
{
    ostringstream oss;
    try
    {
        boost::lock_guard<boost::mutex> lock(hdl_mutex_);
        if(!hdl_.expired()) {
            client::connection_ptr con = ws_client_.get_con_from_hdl(hdl_);
            if(con->get_state() == websocketpp::session::state::open) {
                int close_code = websocketpp::close::status::normal;
                ws_client_.close(hdl_, close_code, "");
                oss << "Notice: close connection.";
            }
        }
    } catch(websocketpp::exception& e) {
        oss << "Error: catch websocketpp exception:[" << e.what() << "]";
    } catch(...) {
        oss << "Error: catch other exception";
    }
    cout << oss.str() << endl;
}

void asr_ws_example::stop_io_service()
{
    ostringstream oss;
    try
    {
        ws_client_.stop();
        oss << "Notice: stop io_service.";
    } catch(websocketpp::exception& e) {
        oss << "Error: catch websocketpp exception:[" << e.what() << "]";
    } catch(...) {
        oss << "Error: catch other exception";
    }
    cout << oss.str() << endl;
}

context_ptr asr_ws_example::on_tls_init(websocketpp::connection_hdl) {
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

void asr_ws_example::on_open(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_open called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
    // 开启发送线程
    boost::thread(boost::bind(&asr_ws_example::work_thread, this)).detach();
}

void asr_ws_example::on_close(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_close called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
}

void asr_ws_example::on_fail(websocketpp::connection_hdl hdl)
{
    ostringstream oss;
    oss << "Notice: on_fail called, hdl:[" << hdl.lock().get() << "]";
    cout << oss.str() << endl;
}

void asr_ws_example::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
    ostringstream oss;
    oss << "Notice: on_message called, " << "hdl:[" << hdl.lock().get()
                                         << "], payload_length:[" << msg->get_payload().length()
                                         << "]";
    cout << oss.str() << endl;
#ifdef __ASR_REALTIME_ENABLE__
    recv_asr_realtime_msg(msg->get_payload());
#else
    recv_asr_msg(msg->get_payload());
#endif
}

void asr_ws_example::work_thread()
{
    FILE* fp = fopen(input_filename_.c_str() , "rb");
    if (fp == NULL) {
        std::cout << "Notice: workThread open file failed!!! filename: " << input_filename_ << std::endl;
        return;
    }
    vector<char> buffer(5120);
    size_t size = 0;

    std::cout << "Notice: workThread begin send request data " << std::endl;
    string json_data;
    int req_idx = 0;
    while (!feof(fp)) {
        // 固定5120 字节，最后一包除外
        size = fread(buffer.data(), 1, 5120, fp);
        if (size <= 0) {
            break;
        }
        asr_params_.req_idx = req_idx++;
        if (feof(fp)) {
            asr_params_.req_idx = 0 - asr_params_.req_idx;
        }
        asr_params_.audio_data = websocketpp::base64_encode((const unsigned char*)buffer.data(), size);
        // 生成json 参数
        json_data = gen_json_request(access_token_, version_, asr_params_);
        // 发送websocket
        send_request_frame(hdl_, json_data);
    }
    std::cout << "Notice: workThread end send request data finish" << std::endl;
    fclose(fp);
}

