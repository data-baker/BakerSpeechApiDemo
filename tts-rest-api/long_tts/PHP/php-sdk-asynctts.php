<?php
//获取访问令牌
$client_secret = '***'; //应用secret
$client_id = '***'; //应用id
$grant_type = 'client_credentials'; //固定格式

//1.获取token
$url = 'https://openapi.data-baker.com/oauth/2.0/token?grant_type='.$grant_type.'&client_id='.$client_id.'&client_secret='.$client_secret;

//curl get请求
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false); //信任任何证书
curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0); // 检查证书中是否设置域名,0不验证
$token_res = curl_exec($ch);

//如果错误 查看错误信息
if(curl_errno($ch))
{
    print curl_error($ch);
}
curl_close($ch);

//进行token信息解析
$token_info = json_decode($token_res,1);
$access_token = $token_info['access_token'];    //获取到的token信息
//var_dump($access_token);die;
//$access_token = '***';

//2.长文本语音合成
$url = 'https://openapi.data-baker.com/asynctts/synthesis/work';

$file_path = './test.txt';
$text = file_get_contents($file_path);
$notify_url = '***/tts_async';

$post_data['access_token'] = $access_token;
$post_data['text'] = $text;
$post_data['notifyUrl'] = $notify_url;
$post_data['voiceName'] = 'Jiaojiao';
$post_data =json_encode($post_data);

$Content_Type = 'application/json; charset=utf-8';
$headers = array(
    'Content-Type:'.$Content_Type
);
$ch = curl_init();
curl_setopt($ch, CURLINFO_HEADER_OUT, true);
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 60); // 识别时长不超过原始音频
curl_setopt($ch, CURLOPT_POSTFIELDS, $post_data);
$res = curl_exec($ch);
if(curl_errno($ch))
{
    print curl_error($ch);
}
curl_close($ch);
//echo(json_decode($res,1)['message']);
var_dump($res);die;




