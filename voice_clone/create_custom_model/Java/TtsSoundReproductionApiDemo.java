package com.databaker.web.tts;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


/**
 * 声音复刻RESTFUL API接口调用示例
 * 附：声音复刻RESTFUL API文档 【https://www.data-baker.com/specs/file/reprint_api_restful】
 *
 * 注意：仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class TtsSoundReproductionApiDemo {

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
     * 声音复刻API地址
     */
    public static String soundReproductionUrl = "https://openapi.data-baker.com/gramophone/v1/submit";

    /**
     * 音频列表，需满足一定条件：
     * 1.格式为pcm或wav
     * 2.采样率为16000hz,位深为16bit，单声道
     * 3.有效时长最好不小于3分钟（接口实际是以识别出的字数作为判定标准）
     */
    public static List<File> originFiles = new ArrayList<>();

    public static void main(String[] args) {
        String accessToken = getAccessToken();
        if (StringUtils.isNotEmpty(accessToken)) {
            doSoundReproduction(accessToken, originFiles, "mobile", "https://openapi.data-baker.com/gramophone/v1/api/notify");
        }
    }

    /**
     * 提交复刻任务
     *
     * 开发者需开发一个回调接口，接口地址作为参数notifyUrl，用来接收模型训练的结果,具体写法可参考接口文档【https://www.data-baker.com/specs/file/reprint_api_restful】中的回调部分
     */
    private static void doSoundReproduction(String accessToken, List<File> originFiles, String mobile, String notifyUrl) {
        //创建连接
        OkHttpClient client = new OkHttpClient();
        //构建requestBody,传入参数
        MultipartBody.Builder requestBody = new MultipartBody.Builder().setType(MultipartBody.FORM);
        for (File file : originFiles) {
            RequestBody body = RequestBody.create(file, MediaType.parse("multipart/form-data"));
            String filename = file.getName();
            requestBody.addFormDataPart("originFiles", filename, body);
        }
        requestBody.addFormDataPart("access_token", accessToken);
        requestBody.addFormDataPart("mobile", mobile);
        requestBody.addFormDataPart("notifyUrl", notifyUrl);

        //构造request
        Request request = new Request.Builder()
                .url(soundReproductionUrl)
                .method("POST", requestBody.build())
                .build();
        try {
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                System.out.println("调用成功，返回结果：" + response.body().string());
            } else {
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
