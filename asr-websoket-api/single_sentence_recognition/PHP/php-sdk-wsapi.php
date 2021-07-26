<?php
    ini_set("max_execution_time", "300");
    require_once "websocket-php/vendor/autoload.php";
    //1.获取token链接
    $client_secret = '***'; //应用secret
    $client_id = '***'; //应用id
    $grant_type = 'client_credentials'; //固定格式
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

    //2.一句话识别请求
    $url = 'wss://openapi.data-baker.com/asr/wsapi/';
    $file_path = './test.wav';//文件路径

    //定义请求方法
    $data_param['access_token'] = $access_token;
    $data_param['version'] = '1.0';
    $data_param['asr_params']['audio_format'] = 'wav';
    $data_param['asr_params']['sample_rate'] = 16000;


    //读取文件 发送数据请求
    $client = new \WebSocket\Client($url); //实例化
    $file_size = filesize($file_path); //获取文件大小
    if(!$file_size) die("文件大小不能为0");
    $package_nums = ceil($file_size/5120); //计算要发送的包数量
    $handle = fopen($file_path,'rb');
    $i=0; //起始音频编号

    while (!feof($handle)){
        if($i == $package_nums-1){
            $data_param['asr_params']['req_idx'] = -$i;
        }else{
            $data_param['asr_params']['req_idx'] = $i;
        }
        $audio_info = fread($handle,5120);
        $data_param['asr_params']['audio_data'] = base64_encode($audio_info);
        $data = json_encode($data_param);
        $client->send($data); //发送数据
        $i++;
    }

    //监听数据
    $flag = true;
    while ($flag) {
        try {
            $message = $client->receive();//获取数据
            $result_info = json_decode($message,1);
            if($result_info['code'] == 90000){
                if($result_info['data']['end_flag']==1){
                    //(说明，根据业务逻辑情况使用，本示例对返回完整信息进行展示)
                    $flag=false;//最后一包数据获取完成 停止获取数据
                    var_dump($result_info['data']['nbest']);
                }
            }else{
                throw new Exception($result_info['message']);
            }

        } catch (\WebSocket\ConnectionException $e) {
            die($e->getMessage());
        }
    }



