package com.databaker.web.tts;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.Charset;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * 在线合成WebSocket API接口调用示例
 * 附：在线合成Websocket API文档 【https://www.data-baker.com/specs/file/tts_api_websocket】
 * <p>
 * 注意：
 * 1.本demo展示了如何保存音频流到本地文件
 * 2.本demo仅完成基本的接口调用，失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class TtsWebSocketDemo extends WebSocketListener {
    /**
     * 授权：需要在开放平台获取【https://ai.data-baker.com/】
     */
    private static final String clientId = "YOUR_CLIENT_ID";
    private static final String clientSecret = "YOUR_CLIENT_SECRET";

    /**
     * 获取token的地址信息
     */
    public static String tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=client_credentials&client_secret=%s&client_id=%s";

    private static final String hostUrl = "wss://openapi.data-baker.com/wss";

    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyy-MM-dd HH:mm:ss.SSS");

    /**
     * 开始时间
     */
    private static ThreadLocal<Date> timeBegin = ThreadLocal.withInitial(() -> new Date());

    /**
     * 结束时间
     */
    private static ThreadLocal<Date> timeEnd = ThreadLocal.withInitial(() -> new Date());

    private Date startTime;

    private String accessToken = getAccessToken();

    /**
     * utf-8编码，不超过1024字节
     */
    private static Integer MAX_TEXT_LENGTH = 255;

    /**
     * 文本
     */
    private String text = "感谢使用标贝科技语音合成服务，祝您使用愉快！";
    /**
     * 发音人
     */
    private String voiceName = "Tiantian";

    /**
     * 保存结果文件的路径，开发者需要根据实际路径调整
     */
    private File resultFile;

    public TtsWebSocketDemo(File resultFile) {
        this.resultFile = resultFile;
    }

    public TtsWebSocketDemo(String text, File resultFile) {
        this.text = text;
        this.resultFile = resultFile;
    }

    public TtsWebSocketDemo(String text, String voiceName, File resultFile) {
        this.text = text;
        this.voiceName = voiceName;
        this.resultFile = resultFile;
    }

    @Override
    public void onOpen(WebSocket webSocket, Response response) {
        super.onOpen(webSocket, response);
        this.startTime = timeBegin.get();
        new Thread(() -> {
            //连接成功，开始发送数据
            //发送文本
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("access_token", accessToken);
            jsonObject.put("version", "1.0");
            //填充asr_params
            JSONObject ttsParams = new JSONObject();
            //domain非必填
            ttsParams.put("domain", "1");
            ttsParams.put("interval", "0");
            ttsParams.put("language", "ZH");
            ttsParams.put("voice_name", voiceName);
            ttsParams.put("text", Base64.getEncoder().encodeToString((text.length() > MAX_TEXT_LENGTH ? text.substring(0, MAX_TEXT_LENGTH) : text).getBytes(Charset.forName("UTF-8"))))
            ;
            jsonObject.put("tts_params", ttsParams);
            System.out.println("dataSent:" + (text.length() > MAX_TEXT_LENGTH ? text.substring(0, MAX_TEXT_LENGTH) : text));
            webSocket.send(jsonObject.toString());

            System.out.println("all data is send");
        }).start();
    }

    @Override
    public void onMessage(WebSocket webSocket, String text) {
        super.onMessage(webSocket, text);
        JSONObject resp = JSON.parseObject(text);
        if (resp != null) {
            if (resp.getInteger("code") != 90000) {
                System.out.println("code=>" + resp.getInteger("code") + " error=>" + resp.getString("message") + " trace_id=" + resp.getString("trace_id"));
                //关闭连接
                webSocket.close(1000, "");
                System.out.println("发生错误，关闭连接");
                return;
            }
            JSONObject dataObject = resp.getJSONObject("data");
            if (dataObject != null) {
                if (StringUtils.isNotEmpty(dataObject.getString("audio_data"))) {
                    //写入文件
                    FileOutputStream out = null;
                    try {
                        out = new FileOutputStream(resultFile, true);
                        byte[] b = Base64.getDecoder().decode(dataObject.getString("audio_data"));
                        out.write(b);
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        if (out != null) {
                            try {
                                out.close();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                }
                if (dataObject.getInteger("end_flag") == 1) {
                    //说明数据全部返回完毕，可以关闭连接，释放资源
                    System.out.println("session end,tts finished. ");
                    System.out.println(sdf.format(startTime) + "开始");
                    System.out.println(sdf.format(timeEnd.get()) + "结束");
                    System.out.println("耗时:" + (timeEnd.get().getTime() - startTime.getTime()) + "ms");
                    webSocket.close(1000, "");
                }
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
        File file = new File("src/main/resources/tts1.pcm");
        //测试文本
        String ttsTestText = "教育部13日召开新闻通气会表示，暑期托管服务主要面向确有需求的家庭和学生，并由家长学生自愿选择参加。托管服务应以看护为主，合理组织提供一些集体游戏活动、文体活动、阅读指导、综合实践、兴趣拓展、作业辅导等服务，但不得组织集体补课、讲授新课。关于暑期托管变成第三学期的说法是不符合实际的。";
        //测试简单调用
        client.newWebSocket(request, new TtsWebSocketDemo(ttsTestText, file));
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
}