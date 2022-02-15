package com.databaker.web.asr;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;

import java.io.File;
import java.io.FileInputStream;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;
/**
 * 声纹识别RESTFUL API接口调用示例
 * 附：声纹识别RESTFUL API文档 【https://www.data-baker.com/specs/file/vpr_api_restful】
 *
 * 注意：
 * 1.仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 * 2.此demo将6个接口一次性封装，请参照接口文档，使用时只需在main中逐一解开方法注释，并补充方法中的参数值即可
 *
 * @author data-baker
 */
public class VoiceprintRecognitionDemo {

    /**
     * 授权：需要在开放平台获取【https://ai.data-baker.com/】
     */
    private static final String clientId = "";
    private static final String clientSecret = "";
    /**
     * 获取token的地址信息
     */
    public static String tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=client_credentials&client_secret=%s&client_id=%s";

    /** 创建声纹ID API地址 */
    private static String createUrl = "https://openapi.data-baker.com/vpr/createid";
    /** 声纹注册API地址 */
    private static String registUrl = "https://openapi.data-baker.com/vpr/register";
    /** 查询声纹状态码API地址 */
    private static String queryStatusUrl = "https://openapi.data-baker.com/vpr/status";
    /** 删除声纹API地址 */
    private static String deleteUrl = "https://openapi.data-baker.com/vpr/delete";
    /** 声纹验证(1:1)API地址 */
    private static String matchUrl = "https://openapi.data-baker.com/vpr/match";
    /** 声纹对比(1:N)API地址 */
    private static String searchUrl = "https://openapi.data-baker.com/vpr/search";

    public static void main(String[] args){
        try {
            /** 创建声纹ID */
//        doCreateLibrary();
            /** 声纹注册 */
//            doRegist();
            /** 查询声纹状态码 */
//        doQueryStatus();
            /** 删除声纹 */
//        doDelete();
            /** 声纹验证(1:1) */
//        doMatch();
            /** 声纹验证(1:N) */
//        doSearch();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void doCreateLibrary() {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        String rightResultdesc = "创建声纹库完成，结果信息";
        String wrongResultdesc = "创建声纹库失败，结果信息";
        sendReqUtil(params,createUrl,rightResultdesc,wrongResultdesc);
    }

    public static void doRegist() throws Exception {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        params.put("format","pcm");
        params.put("audio",encodeBase64File(""));  //获取音频数据地址
        params.put("registerId","");                    //声纹库id
        params.put("name","声纹注册测试");                //自定义名字
        params.put("scoreThreshold", 65.0);             //注册有效分数
        String rightResultdesc = "声纹注册完成，结果信息";
        String wrongResultdesc = "声纹注册失败，错误信息";
        sendReqUtil(params,registUrl,rightResultdesc,wrongResultdesc);
    }

    private static void doQueryStatus() {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        params.put("registerId","");                    //声纹库id
        String rightResultdesc = "查询声纹状态码完成，结果信息";
        String wrongResultdesc = "查询声纹状态码失败，结果信息";
        sendReqUtil(params,queryStatusUrl,rightResultdesc,wrongResultdesc);
    }

    private static void doDelete() {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        params.put("registerId","");                    //声纹库id
        String rightResultdesc = "删除声纹完成，结果信息";
        String wrongResultdesc = "删除声纹失败，结果信息";
        sendReqUtil(params,deleteUrl,rightResultdesc,wrongResultdesc);
    }

    public static void doMatch() throws Exception {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        params.put("format","pcm");
        params.put("audio",encodeBase64File("")); //获取音频数据地址
        params.put("scoreThreshold",30.0);             //分数阈值
        params.put("matchId","");                      //声纹库id
        String rightResultdesc = "声纹1:1验证完成，结果信息";
        String wrongResultdesc = "声纹1:1验证失败，结果信息";
        sendReqUtil(params,matchUrl,rightResultdesc,wrongResultdesc);
    }

    public static void doSearch() throws Exception {
        Map<String,Object> params = new HashMap<>();
        params.put("access_token",getAccessToken());
        params.put("format","pcm");
        params.put("audio",encodeBase64File("")); //获取音频数据地址
        params.put("scoreThreshold",30.0);             //分数阈值
        params.put("listNum",5);
        String rightResultdesc = "声纹1:N验证完成，结果信息";
        String wrongResultdesc = "声纹1:N验证失败，结果信息";
        sendReqUtil(params,searchUrl,rightResultdesc,wrongResultdesc);
    }

    public static String encodeBase64File(String path) throws Exception {
        File file = new File(path);
        FileInputStream inputFile = new FileInputStream(file);
        byte[] buffer = new byte[(int) file.length()];
        inputFile.read(buffer);
        inputFile.close();
        return Base64.getEncoder().encodeToString(buffer);
    }

    private static void sendReqUtil(Map<String,Object> params,String url,String rightResultdesc,String wrongResultdesc){
        try {
            OkHttpClient client = new OkHttpClient().newBuilder()
                    .connectTimeout(60, TimeUnit.SECONDS)
                    .readTimeout(60, TimeUnit.SECONDS)
                    .writeTimeout(60, TimeUnit.SECONDS)
                    .build();
            MediaType mediaType = MediaType.parse("application/json");
            RequestBody body = RequestBody.create(JSONObject.toJSONString(params),mediaType);
            //构造request
            Request request = new Request.Builder()
                    .url(url)
                    .method("POST", body)
                    .addHeader("Content-Type", "application/json")
                    .build();
            Response response = null;
            response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                JSONObject jsonObject = JSON.parseObject(response.body().string());
                System.out.println(rightResultdesc + ": " + (jsonObject == null ? "" : jsonObject));
            } else {
                System.out.println(wrongResultdesc + ": " + response.body().string());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static String getAccessToken() {
        String accessToken = "";
        OkHttpClient client = new OkHttpClient();
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
