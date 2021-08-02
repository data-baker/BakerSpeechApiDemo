package com.databaker.web.asr;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.FileInputStream;


/**
 * 录音文件识别RESTFUL API接口调用示例
 * 附：录音文件识别RESTFUL API文档 【https://www.data-baker.com/specs/file/asr_file_api_restful】
 *
 * 注意：
 * 1.仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 * 2.录音文件识别中的录音文件有两种提交方式：A.在header中通过音频下载链接方式提交 B.在body中通过音频文件流方式提交。服务端优先使用方式A提供的音频。
 * 3.录音文件识别是非实时接口，提供了两种方式获取结果：A.开发者提供回调接口 B.开发者主动查询进度接口。开发者可自由选择，主动查询时注意频率不要太高。
 *
 * @author data-baker
 */
public class AsrFileRestApiDemo {
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
     * 录音文件识别API地址
     */
    public static String asrSubmitTaskUrl = "https://openapi.data-baker.com/asr/starttask";
    /**
     * 录音文件识别API地址
     */
    public static String asrQueryTaskUrl = "https://openapi.data-baker.com/asr/taskresult";
    /**
     * 文件流方式上传音频，最大支持64M；音频URL方式上传，最大支持512M
     */
    public static Integer MAX_FILE_SIZE = 64 * 1024 * 1024;

    /**
     * 仅作为demo示例
     * 失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
     **/
    public static void main(String[] args) {
        String accessToken = getAccessToken();
        if (StringUtils.isNotEmpty(accessToken)) {
            //支持两种上传音频方式：url或文件流；支持三种音频格式：pcm、wav、mp3
            //1.url方式
            //String audioFileInfo = "YOUR_AUDIO_FILE_URL";
            //String taskId = submitFileAsrTask(accessToken, audioFileInfo, AudioFileTypeEnum.UrlType, "mp3", 16000, "");

            //2.文件流方式
            String audioFileInfo = "YOUR_AUDIO_FILE_FULL_PATH";
            String taskId = submitFileAsrTask(accessToken, audioFileInfo, AudioFileTypeEnum.FileType, "pcm", 16000, "");

            //查询识别结果
            try {
                if (StringUtils.isNotEmpty(taskId)) {
                    Thread.sleep(10 * 1000);
                    queryFileAsrTaskResult(accessToken, taskId);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 提交录音文件识别任务
     */
    public static String submitFileAsrTask(String accessToken, String audioFileInfo, AudioFileTypeEnum fileType, String audioFormat, Integer sampleRate, String callback_url) {
        try {
            OkHttpClient client = new OkHttpClient();
            MediaType mediaType = MediaType.parse("application/octet-stream");
            //requestBody默认为空
            RequestBody requestBody = RequestBody.create(null, new byte[0]);
            //如果以文件流方式上传，文件流放在requestBody里面
            if (AudioFileTypeEnum.FileType.equals(fileType)) {
                File audioFile = new File(audioFileInfo);
                if (!audioFile.exists() || audioFile.length() > MAX_FILE_SIZE) {
                    return "";
                }
                FileInputStream in = new FileInputStream(audioFile);
                byte[] fileByte = new byte[(int) audioFile.length()];
                int realLen = in.read(fileByte);
                //确保音频文件内容全部被读取
                if (realLen == (int) audioFile.length()) {
                    requestBody = RequestBody.create(mediaType, fileByte);
                }
            }
            //构造完整request
            Request request = new Request.Builder()
                    .url(asrSubmitTaskUrl)
                    .addHeader("access_token", accessToken)
                    .addHeader("file_url", AudioFileTypeEnum.UrlType.equals(fileType) ? audioFileInfo : "")
                    .addHeader("audio_format", audioFormat)
                    .addHeader("sample_rate", String.valueOf(sampleRate))
                    .addHeader("domain", "common")
                    .addHeader("callback_url", callback_url)
                    .method("POST", requestBody)
                    .build();
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                JSONObject jsonObject = JSON.parseObject(response.body().string());
                System.out.println("提交识别任务成功，任务id：" + (jsonObject == null ? "" : jsonObject.getString("taskid")));
                return jsonObject == null ? "" : jsonObject.getString("taskid");
            } else {
                System.out.println("提交识别任务失败，错误信息：" + response.body().string());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * 查询录音文件识别任务的结果
     */
    public static void queryFileAsrTaskResult(String accessToken, String taskId) {
        try {
            OkHttpClient client = new OkHttpClient();
            //requestBody默认为空
            RequestBody requestBody = RequestBody.create(null, new byte[0]);
            Request request = new Request.Builder()
                    .url(asrQueryTaskUrl)
                    .addHeader("access_token", accessToken)
                    .addHeader("taskid", taskId)
                    .method("POST", requestBody)
                    .build();
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                JSONObject jsonObject = JSON.parseObject(response.body().string());
                // 返回的结构开发者可参考接口文档【https://www.data-baker.com/specs/file/asr_file_api_restful】中的“响应识别结果”部分。
                // 本demo测试时使用的是单声道音频，故仅获取left_result作为示例。开发者请根据音频情况自行解析。
                System.out.println("查询识别任务成功，结果信息：" + (jsonObject == null ? "" : jsonObject.getString("left_result")));
            } else {
                System.out.println("查询识别任务失败，错误信息：" + response.body().string());
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

    public enum AudioFileTypeEnum {
        UrlType(1),
        FileType(2);

        private int type;

        AudioFileTypeEnum(int type) {
            this.type = type;
        }
    }
}
