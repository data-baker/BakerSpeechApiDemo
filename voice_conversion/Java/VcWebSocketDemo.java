package com.databaker.web.vc;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import okio.ByteString;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.ArrayUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * 声音转换WebSocket API接口调用示例
 * 附：声音转换Websocket API文档 【https://www.data-baker.com/specs/file/vc_api_websocket】
 * <p>
 * 注意：仅作为demo示例,失败重试、token过期重新获取、日志打印等优化工作需要开发者自行完成
 *
 * @author data-baker
 */
public class VcWebSocketDemo extends WebSocketListener {
    /**
     * 授权：需要在开放平台获取【https://ai.data-baker.com/】
     */
    private static final String clientId = "YOUR_CLIENT_ID";
    private static final String clientSecret = "YOUR_CLIENT_SECRET";

    /**
     * 获取token的地址信息
     */
    public static String tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token?grant_type=client_credentials&client_secret=%s&client_id=%s";

    private static final String hostUrl = "wss://openapi.data-baker.com/ws/voice_conversion";

    /**
     * 文件路径【开发者需要根据实际路径调整。支持的音频编码格式:PCM,采样率16K,位深16bit,中文普通话,时长不超过180分钟】
     */
    private static final String pcmFile = "/home/asr/16k_16bit.pcm";
    /**
     * 文件夹路径【声音转换后文件存放地址，开发者需要根据实际路径调整。】
     */
    private static final String dic = "/home/asr";

    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyy-MM-dd HH:mm:ss.SSS");

    // 开始时间
    private static ThreadLocal<Date> timeBegin = ThreadLocal.withInitial(() -> new Date());

    // 结束时间
    private static ThreadLocal<Date> timeEnd = ThreadLocal.withInitial(() -> new Date());

    // 发音人
    private String voiceName = "Vc_baklong";

    private Date startTime;

    private String accessToken = getAccessToken();

    @Override
    public void onOpen(WebSocket webSocket, Response response) {
        super.onOpen(webSocket, response);
        this.startTime = timeBegin.get();
        // 该demo直接从文件中读取音频流【实际场景可能是实时从麦克风获取音频流，开发者自行修改获取音频流的逻辑即可】
        new Thread(() -> {
            File file = new File(pcmFile);
            // 连接成功，开始发送数据
            // 第二部分是一个JSON的字符串
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("access_token", accessToken);
            jsonObject.put("voice_name", voiceName);
            jsonObject.put("enable_vad", true);
            jsonObject.put("align_input", true);
            jsonObject.put("lastpkg", false);

            int length = jsonObject.toJSONString().getBytes().length;
            // 第一部分
            byte[] b = new byte[4];
            b[0] = (byte) (length >> 24 & 0xFF);
            b[1] = (byte) (length >> 16 & 0xFF);
            b[2] = (byte) (length >> 8 & 0xFF);
            b[3] = (byte) (length & 0xFF);

            FileInputStream fileInputStream = null;
            ByteArrayOutputStream byteOut = null;
            try {
                fileInputStream = new FileInputStream(file);
                byteOut = new ByteArrayOutputStream();
                int size = 32000;
                byte[] byteArray = new byte[size];
                int totalLength = 0;
                int read = 0;
                // 发送音频
                while ((read = fileInputStream.read(byteArray, 0, size)) > 0) {
                    totalLength += read;
                    System.out.println();
                    if (read == size && totalLength < file.length()) {
                        byte[] bytes = ArrayUtils.addAll(ArrayUtils.addAll(b, jsonObject.toJSONString().getBytes()), byteArray);
                        webSocket.send(new ByteString(bytes));
                        Thread.sleep(40);
                    } else {
                        // 最后一包
                        jsonObject.put("lastpkg", true);
                        length = jsonObject.toJSONString().getBytes().length;
                        b[0] = (byte) (length >> 24 & 0xFF);
                        b[1] = (byte) (length >> 16 & 0xFF);
                        b[2] = (byte) (length >> 8 & 0xFF);
                        b[3] = (byte) (length & 0xFF);
                        byte[] subarray = ArrayUtils.subarray(byteArray, 0, read);
                        byte[] bytes = ArrayUtils.addAll(ArrayUtils.addAll(b, jsonObject.toJSONString().getBytes()), subarray);
                        webSocket.send(new ByteString(bytes));
                    }
                }
                System.out.println("all data is send");
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (fileInputStream != null) {
                    try {
                        fileInputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                if (byteOut != null) {
                    try {
                        byteOut.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }

    @Override
    public void onMessage(WebSocket webSocket, String text) {
        super.onMessage(webSocket, text);
    }

    @Override
    public void onMessage(WebSocket webSocket, ByteString bytes) {
        super.onMessage(webSocket, bytes);
        byte[] byteArray = bytes.toByteArray();
        // byte[]转length
        int length = (byteArray[0] << 24) + (byteArray[1] << 16) + (byteArray[2] << 8) + byteArray[3];
        byte[] jsonArray = ArrayUtils.subarray(byteArray, 4, 4 + length);
        String jsonStr = new String(jsonArray);
        JSONObject jsonObject = JSONObject.parseObject(jsonStr);
        byte[] subarray = ArrayUtils.subarray(byteArray, 4 + length, byteArray.length);
        File resultPcmFile = new File(dic, new File(pcmFile).getName().replace(".pcm", "_result.pcm"));
        if (!resultPcmFile.exists()) {
            try {
                resultPcmFile.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            // 写入文件
            FileOutputStream outputStream = new FileOutputStream(resultPcmFile, true);
            IOUtils.write(subarray, outputStream);
            IOUtils.closeQuietly(outputStream);
            if ((int) jsonObject.get("errcode") == 0) {
                if (jsonObject.getBoolean("lastpkg")) {
                    //说明数据全部返回完毕，可以关闭连接，释放资源
                    System.out.println("session end ");
                    System.out.println(sdf.format(startTime) + "开始");
                    System.out.println(sdf.format(timeEnd.get()) + "结束");
                    System.out.println("耗时:" + (timeEnd.get().getTime() - startTime.getTime()) + "ms");
                    System.out.println("本次识别traceId ==》" + jsonObject.getString("traceid"));
                    webSocket.close(1000, "");
                }
            } else {
                System.out.println("errCode=>" + jsonObject.getInteger("errcode") + " errMsg=>" + jsonObject.getString("errmsg") + " traceId=" + jsonObject.getString("traceid"));
                // 关闭连接
                webSocket.close(1000, "");
                System.out.println("发生错误，关闭连接");
                return;
            }
        } catch (Exception e) {
            e.printStackTrace();
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

    public static void main(String[] args) {
        OkHttpClient client = new OkHttpClient.Builder().build();
        Request request = new Request.Builder().url(hostUrl).build();
        client.newWebSocket(request, new VcWebSocketDemo());
    }

    public static String getAccessToken() {
        String accessToken = "";
        OkHttpClient client = new OkHttpClient();
        // request 默认是get请求
        String url = String.format(tokenUrl, clientSecret, clientId);
        Request request = new Request.Builder().url(url).build();
        JSONObject jsonObject;
        try {
            Response response = client.newCall(request).execute();
            if (response.isSuccessful()) {
                // 解析
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
