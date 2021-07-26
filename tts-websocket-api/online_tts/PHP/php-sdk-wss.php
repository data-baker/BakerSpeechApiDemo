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

    //2.在线合成
    $url = 'wss://openapi.data-baker.com/wss';
    $file_path = './test.txt';//文件路径 必须utf-8编码

    //定义请求方法
    $data_param['access_token'] = $access_token;
    $data_param['version'] = '1.0';
    $data_param['tts_params']['domain'] = '1';
    $data_param['tts_params']['language'] = 'ZH';
    $data_param['tts_params']['voice_name'] = 'Jiaojiao';

    //发送数据请求
    $client = new \WebSocket\Client($url); //实例化
    $handle = fopen($file_path,'rb');
    while (!feof($handle)){
        $audio_info = fread($handle,2048);
        $data_param['tts_params']['text'] = base64_encode($audio_info);
        $data = json_encode($data_param);
        $client->send($data); //发送数据
    }

    //监听数据
    $info_list = [];
    $flag = true;
    while ($flag) {
        try {
            $message = $client->receive();//获取数据
            $result_info = json_decode($message,1);
            if($result_info['code'] == 90000){
                $info_list[$result_info['data']['idx']] = $result_info['data'];
            }else{
                throw new Exception($result_info['message']);
            }
            if($result_info['data']['end_flag']==1) $flag=false;//最后一包数据获取完成 停止获取数据
            //var_dump($result_info);
        } catch (\WebSocket\ConnectionException $e) {
            die($e->getMessage());
        }
    }
    //对获取数据进行处理 (说明，根据业务逻辑情况使用，本示例对返回信息一次性写入)
    if($info_list){
        ksort($info_list);//进行升序排序（说明，酌情使用，本示例对返回识别段落进行了二次排序）
        $file_name = './test.pcm';
        $handle=fopen($file_name,"w");
        foreach ($info_list as $v){
            //写入文件
            fwrite($handle, base64_decode($v['audio_data']));
        }
        die("识别音频保存完成");
    }else{
        die("获取的数据为空");
    }

