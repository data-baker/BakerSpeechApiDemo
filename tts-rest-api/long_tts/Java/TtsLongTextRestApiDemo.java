package com.databaker.web.tts;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;

/**
 * 长文本语音合成RESTFUL API接口调用示例
 * 附：长文本语音合成RESTFUL API文档 【https://www.data-baker.com/specs/file/tts_api_long_text】
 * <p>
 * 注意：仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class TtsLongTextRestApiDemo {
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
     * 长文本合成API地址
     */
    public static String ttsLongTextUrl = "https://openapi.data-baker.com/asynctts/synthesis/work";

    public static void main(String[] args) {
        String accessToken = getAccessToken();
        if (StringUtils.isNotEmpty(accessToken)) {
            doSynthesis(accessToken, "Nannan", "测试文本", 6, 5.0, 5.0, "https://openapi.data-baker.com/asynctts/synthesis/notify");
        }
    }

    /**
     * 实际合成逻辑
     *
     * 开发者需开发一个回调接口，接口地址作为参数notifyUrl，用来接收合成的音频链接,具体写法可参考接口文档【https://www.data-baker.com/specs/file/tts_api_long_text】中的回调部分
     */
    public static void doSynthesis(String accessToken, String voiceName, String text, Integer audioType, Double speed, Double volume, String notifyUrl) {
        OkHttpClient client = new OkHttpClient();
        MediaType mediaType = MediaType.parse("application/json");
        //构造requestBody
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("access_token", accessToken);
        jsonObject.put("voiceName", voiceName);
        jsonObject.put("text", text);
        jsonObject.put("volume", volume);
        jsonObject.put("speed", speed);
        jsonObject.put("audiotype", audioType);
        jsonObject.put("notifyUrl", notifyUrl);
        RequestBody body = RequestBody.create(mediaType, jsonObject.toJSONString());
        //构造request
        Request request = new Request.Builder()
                .url(ttsLongTextUrl)
                .method("POST", body)
                .addHeader("Content-Type", "application/json")
                .build();
        try {
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                //1.如果请求正常，会返回作品id，开发者可根据业务需要自行解析处理
                //2.实际的音频链接，会通过回调接口通知开发者
                System.out.println("调用成功，返回结果：" + response.body().string());
            } else {
                //处理错误，主要的错误类别：Token失效、文本长度过长、使用了授权范围外的音色等
                System.out.println("调用失败，返回结果：" + response.body().string());
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
