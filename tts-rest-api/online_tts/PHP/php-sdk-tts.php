<?php
//获取访问令牌
$client_secret = '***'; //应用secret
$client_id = '***'; //应用id
$grant_type = 'client_credentials'; //固定格式

//1.获取token链接
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

//2.在线合成 说明（支持get post方式，本demo使用get方式）
$url = 'https://openapi.data-baker.com/tts?access_token='.$access_token.'&text="PHP是世界上最好的语言，不接受任何反驳"&domain=1&language=zh&voice_name=Jiaojiao';

//curl get请求
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
$audio_info = curl_exec($ch);

//获取请求链接响应头信息
$response_header = curl_getinfo($ch);
if(stripos($response_header['content_type'],'audio') ===false){
    var_dump('合成内容失败');die;
}
//进行合成内容保存
$file_name = './test.mp3';
$res = file_put_contents($file_name,$audio_info);
if($res){
    var_dump('文件保存成功');die;
}




