package com.databaker.web.asr;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.FileInputStream;

/**
 * （一句话）在线识别RESTFUL API接口调用示例
 * 附：在线识别RESTFUL API文档 【https://www.data-baker.com/specs/file/asr_word_api_restful】
 * <p>
 * 注意：仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class AsrRestApiDemo {
    /**
     * 授权：需要在开放平台获取【https://ai.data-baker.com/】
     */
    private static final String clientId = "YOUR_CLIENT_ID";
    private static final String clientSecret = "YOUR_CLIENT_SECRET";

    /**
     * 获取token的地址信息
     */
    public static String tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=client_credentials&client_secret=%s&client_id=%s";
    /**
     * 一句话识别API地址
     */
    public static String asrUrl = "https://openapi.data-baker.com/asr/api";
    /**
     * 音频文件
     */
    public static String audioPath = "/home/asr/16bit_16k.pcm";
    /**
     * 文件大小限制：开发者需注意服务端会校验音频时长不超过60S。demo作为示例，简化为只校验文件大小
     *
     * @param args
     */
    public static Integer MAX_FILE_SIZE = 10 * 1024 * 1024;

    public static void main(String[] args) {
        String accessToken = getAccessToken();
        if (StringUtils.isNotEmpty(accessToken)) {
            File audioFile = new File(audioPath);
            //一句话在线识别支持的音频长度在60S内，开发者需注意音频流的大小
            if (audioFile.exists() && audioFile.length() < MAX_FILE_SIZE) {
                //支持pcm和wav格式:如果是wav格式，audioFormat设置为"wav";如果是pcm格式，audioFormat设置为"pcm"
                doSpeechRecognition(accessToken, audioFile, "pcm", 16000);
            }
        }
    }

    public static void doSpeechRecognition(String accessToken, File audioFile, String audioFormat, Integer sampleRate) {
        try {
            OkHttpClient client = new OkHttpClient();
            MediaType mediaType = MediaType.parse("application/octet-stream");
            FileInputStream in = new FileInputStream(audioFile);
            byte[] fileByte = new byte[(int) audioFile.length()];
            int realLen = in.read(fileByte);
            //确保音频文件内容全部被读取
            if (realLen == (int) audioFile.length()) {
                RequestBody body = RequestBody.create(mediaType, fileByte);
                //构造request
                Request request = new Request.Builder()
                        .url(asrUrl)
                        .addHeader("access_token", accessToken)
                        .addHeader("audio_format", audioFormat)
                        .addHeader("sample_rate", String.valueOf(sampleRate))
                        .addHeader("domain", "common")
                        .method("POST", body)
                        .build();
                Response response = client.newCall(request).execute();
                if (response.isSuccessful()) {
                    JSONObject jsonObject = JSON.parseObject(response.body().string());
                    System.out.println("识别成功,识别结果：" + (jsonObject == null ? "" : jsonObject.getString("text")));
                } else {
                    System.out.println("识别失败，错误信息：" + response.body().string());
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
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
