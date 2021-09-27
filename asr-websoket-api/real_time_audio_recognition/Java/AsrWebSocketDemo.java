package com.databaker.web.asr;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Base64;
import java.util.Date;

/**
 * 流式语音识别 webSocket接口调用示例
 * 附1：一句话识别Websocket API文档 【https://www.data-baker.com/specs/file/asr_word_api_websocket】
 * 附2：长语音识别Websocket API文档 【https://www.data-baker.com/specs/file/asr_stream_websocket】
 *
 * 注意：
 * 1.作为demo,进行了一些简化,采用直接从文件中读取的方式获取音频流,实际场景很可能是从麦克风获取音频流。
 * 2.一句话识别Websocket和长语音识别Websocket使用方式类似,主要区别在于时长限制。
 * 3.如果从麦克风获取音频流,请注意每次发送的数据流大小为52K,不足时补静音段。
 * 4.本demo仅完成基本的接口调用，失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成。
 *
 * @author data-baker
 */

public class AsrWebSocketDemo extends WebSocketListener {
    /**
     * 授权：需要在开放平台获取【https://ai.data-baker.com/】
     */
    private static final String clientId = "YOUR_CLIENT_ID";
    private static final String clientSecret = "YOUR_CLIENT_SECRET";

    /**
     * 获取token的地址信息
     */
    public static String tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=client_credentials&client_secret=%s&client_id=%s";

    private static final String hostUrl = "wss://openapi.data-baker.com/asr/realtime";
    /**
     * 文件路径【开发者需要根据实际路径调整。支持的音频编码格式:PCM(无压缩的PCM文件或WAV文件),采样率8K或16K,位深16bit,单声道】
     */
    private static final String file = "/home/asr/16k_16bit.pcm";


    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyy-MM-dd HH:mm:ss.SSS");

    // 开始时间
    private static ThreadLocal<Date> timeBegin = ThreadLocal.withInitial(() -> new Date());

    // 结束时间
    private static ThreadLocal<Date> timeEnd = ThreadLocal.withInitial(() -> new Date());

    private Date startTime;

    private String accessToken = getAccessToken();

    private StringBuilder resultAsr = new StringBuilder();

    @Override
    public void onOpen(WebSocket webSocket, Response response) {
        super.onOpen(webSocket, response);
        this.startTime = timeBegin.get();
        //该demo直接从文件中读取音频流【实际场景可能是实时从麦克风获取音频流，开发者自行修改获取音频流的逻辑即可】
        new Thread(() -> {
            //连接成功，开始发送数据
            //每一帧音频的大小,固定值,具体参考接口文档
            int frameSize = 5120;
            int interval = 40;
            // 音频的序号
            int req_idx = 0;
            try (FileInputStream fs = new FileInputStream(file)) {
                byte[] buffer = new byte[frameSize];
                // 发送音频
                while (true) {
                    int len = fs.read(buffer);
                    if (len < frameSize) {
                        //文件已读完
                        req_idx = -1 - req_idx;
                    }
                    //发送音频
                    JSONObject jsonObject = new JSONObject();
                    jsonObject.put("access_token", accessToken);
                    jsonObject.put("version", "1.0");
                    //填充asr_params
                    JSONObject asrParams = new JSONObject();
                    //domain非必填
                    asrParams.put("domain", "common");
                    asrParams.put("audio_format", "pcm");
                    asrParams.put("sample_rate", 16000);
                    asrParams.put("req_idx", req_idx);
                    asrParams.put("audio_data", Base64.getEncoder().encodeToString(Arrays.copyOf(buffer, len)));
                    jsonObject.put("asr_params", asrParams);
                    webSocket.send(jsonObject.toString());
                    if (req_idx >= 0) {
                        req_idx++;
                    }else {
                        break;
                    }
                    //模拟音频采样延时【如果从麦克风获取音频流，可删除这句代码】
                    Thread.sleep(interval);
                }
                System.out.println("all data is send");
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }).start();
    }

    @Override
    public void onMessage(WebSocket webSocket, String text) {
        super.onMessage(webSocket, text);
        ResponseData resp = JSON.parseObject(text, ResponseData.class);
        if (resp != null) {
            if (resp.getCode() != 90000) {
                System.out.println("code=>" + resp.getCode() + " error=>" + resp.getMessage() + " trace_id=" + resp.getTrace_id());
                //关闭连接
                webSocket.close(1000, "");
                System.out.println("发生错误，关闭连接");
                return;
            }
            if (resp.getAsr_text() != null) {
                if (resp.getSentence_end()) {
                    resultAsr.append(resp.getAsr_text());
                    System.out.println("当前句子识别结束，识别结果 ==》" + resp.getAsr_text());
                } else {
                    System.out.println("当前句子识别未结束，中间识别结果 ==》" + resp.getAsr_text());
                }
            }
            if (resp.getEnd_flag() == 1) {
                //说明数据全部返回完毕，可以关闭连接，释放资源
                System.out.println("session end ");
                System.out.println(sdf.format(startTime) + "开始");
                System.out.println(sdf.format(timeEnd.get()) + "结束");
                System.out.println("耗时:" + (timeEnd.get().getTime() - startTime.getTime()) + "ms");
                System.out.println("最终识别结果 ==》" + resultAsr.toString());
                System.out.println("本次识别traceId ==》" + resp.getTrace_id());
                webSocket.close(1000, "");
            } else {
                // todo 根据返回的数据处理
            }
        }
    }

    @Override
    public void onFailure(WebSocket webSocket, Throwable t, Response response) {
        super.onFailure(webSocket, t, response);
        try {
            if (null != response) {
                int code = response.code();
                System.out.println("onFailure code:" + code);
                System.out.println("onFailure body:" + response.body().string());
                if (101 != code) {
                    System.out.println("connection failed");
                    System.exit(0);
                }
            }
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * 测试方法
     *
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        OkHttpClient client = new OkHttpClient.Builder().build();
        Request request = new Request.Builder().url(hostUrl).build();
        client.newWebSocket(request, new AsrWebSocketDemo());
    }

    public static String getAccessToken() {
        String accessToken = "";
        OkHttpClient client = new OkHttpClient();
        //request 默认是get请求
        String url = String.format(tokenUrl, clientSecret, clientId);
        Request request = new Request.Builder().url(url).build();
        JSONObject jsonObject;
        try {
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                //解析
                String resultJson = response.body().string();
                jsonObject = JSON.parseObject(resultJson);
                accessToken = jsonObject.getString("access_token");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return accessToken;
    }

    public static class ResponseData {
        /**
         * 状态码（4xxxx表示客户端参数错误，5xxxx表示服务端内部错误）
         */
        private Integer code;
        /**
         * 错误描述
         */
        private String message;
        /**
         * 任务id
         */
        private String trace_id;
        /**
         * 识别结果（code为90000时包含有效数据）
         */
        private String asr_text;
        /**
         * 句子id，从1递增
         */
        private Integer sentence_id;
        /**
         * 句子结束标志
         */
        private Boolean sentence_end;

        /**
         * 是否是最后一个数据块（0：否，1：是）
         */
        private Integer end_flag;


        public Integer getCode() {
            return code;
        }

        public void setCode(Integer code) {
            this.code = code;
        }

        public String getMessage() {
            return message;
        }

        public void setMessage(String message) {
            this.message = message;
        }

        public String getTrace_id() {
            return trace_id;
        }

        public void setTrace_id(String trace_id) {
            this.trace_id = trace_id;
        }

        public String getAsr_text() {
            return asr_text;
        }

        public void setAsr_text(String asr_text) {
            this.asr_text = asr_text;
        }

        public Integer getSentence_id() {
            return sentence_id;
        }

        public void setSentence_id(Integer sentence_id) {
            this.sentence_id = sentence_id;
        }

        public Boolean getSentence_end() {
            return sentence_end;
        }

        public void setSentence_end(Boolean sentence_end) {
            this.sentence_end = sentence_end;
        }

        public Integer getEnd_flag() {
            return end_flag;
        }

        public void setEnd_flag(Integer end_flag) {
            this.end_flag = end_flag;
        }
    }
}