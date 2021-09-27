package com.databaker.web.tts;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.IOException;
import java.net.URLEncoder;
import java.nio.file.Files;

/**
 * 语音合成（个性化音色）RESTFUL API接口调用示例
 * 附：语音合成（个性化音色）RESTFUL API文档 【https://www.data-baker.com/specs/file/reprint_api_compose】
 *
 * 注意：仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class TtsPersonalRestApiDemo {
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
     * 合成使用的地址信息，rate、language等参数在本demo固定，开发者如需调整，参考https://www.data-baker.com/specs/file/reprint_api_compose
     */
    public static String ttsUrl = "https://openapi.data-baker.com/tts_personal?access_token=%s&domain=1&audiotype=%s&language=zh&voice_name=%s&speed=%s&volume=%s&text=%s";

    /**
     * 仅作为demo示例
     * 失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
     **/
    public static void main(String[] args) {
        String accessToken = getAccessToken();
        if (StringUtils.isNotEmpty(accessToken)) {
            //此处voiceName应传入已成功完成复刻过程的modelId(示例：YOUR_MODEL_ID = 5c97a06ce40cf69e33d2d05c25d4807f9f657)
            doSynthesis(accessToken, "YOUR_MODEL_ID", "测试文本", 6, 5.0, 5.0, "/home/tts/test.wav");
        }
    }

    public static void doSynthesis(String accessToken, String voiceName, String originText, Integer audioType, Double speed, Double volume, String filePath) {
        //在非浏览器上操作，需要把合成文本转化为utf-8格式
        try {
            originText = URLEncoder.encode(originText, "utf-8");
            String synthesisUrl = String.format(ttsUrl, accessToken, audioType, voiceName, speed, volume, originText);
            fetchTtsResponse(synthesisUrl, filePath);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 请求并获取音频流保存至本地文件：这里filePath为全路径
     *
     * @param url
     * @param filePath
     * @throws IOException
     */
    public static void fetchTtsResponse(String url, String filePath) throws IOException {
        OkHttpClient client = new OkHttpClient();
        //request 默认是get请求
        Request request = new Request.Builder().url(url).build();
        try {
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                if (response.body() != null
                        && response.body().contentType().toString().startsWith("audio")) {
                    //写入文件
                    File targetFile = new File(filePath);
                    Files.write(targetFile.toPath(), response.body().bytes());
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
