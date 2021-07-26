<?php
//官网在线合成长文本语音合成API SDK 回调接口
public function tts_async(){
    $audio_info = file_get_contents("php://input");
    $audio_arr = json_decode($audio_info,1);
    //对返回结果进行判断
    if(isset($audio_arr['audioUrl'])){
        //1.建议返回信息存储到日志文件 或者 本地文件，自行完成
        //2.对音频链接进行读取写入文件操作，自行完成（请求合成接口不设置audiotype参数，默认返回mp3格式）
        //3.对回调接口返回成功信息 return "<xml><return_code>SUCCESS</return_code><return_msg>OK</return_msg></xml>";
    }

}