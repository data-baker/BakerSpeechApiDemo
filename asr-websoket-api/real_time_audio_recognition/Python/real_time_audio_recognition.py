import argparse
import json
import base64
import time
from threading import Thread

import requests
import websocket


class Client:
    def __init__(self, data, uri):
        self.data = data
        self.uri = uri

    #建立连接
    def connect(self):
        ws_app = websocket.WebSocketApp(uri,
                                        on_open=self.on_open,
                                        on_message=self.on_message,
                                        on_error=self.on_error,
                                        on_close=self.on_close)
        ws_app.run_forever()

    # 建立连接后发送消息
    def on_open(self, ws):
        def run(*args):
            for i in range(len(self.data)):
                ws.send(self.data[i])

        Thread(target=run).start()


    # 接收消息
    def on_message(self, ws, message):
        code = json.loads(message).get("code")
        if code != 90000:
            # 打印接口错误
            print(message)
        if json.loads(message).get('sentence_end') == "true":
            print(json.loads(message).get('asr_text'))

    # 打印错误
    def on_error(slef, ws, error):
        print("error: ", str(error))

    # 关闭连接
    def on_close(ws):
        print("client closed.")


# 准备数据
def prepare_data(args, access_token):
    # 读取音频文件
    with open(args.file_path, 'rb') as f:
        file = f.read()

    # 填写Header信息
    audio_format = args.audio_format
    sample_rate = args.sample_rate

    splited_data = [str(base64.b64encode(file[i:i + 5120]), encoding='utf-8') for i in range(0, len(file), 5120)]
    asr_params = {"audio_format": audio_format, "sample_rate": int(sample_rate), "speech_type": 1}

    json_list = []
    for i in range(len(splited_data)):
        if i != len(splited_data) - 1:
            asr_params['req_idx'] = i
        else:
            asr_params['req_idx'] = -len(splited_data) + 1
        asr_params["audio_data"] = splited_data[i]
        data = {"access_token": access_token, "version": "1.0", "asr_params": asr_params}

        json_list.append(json.dumps(data))

    return json_list


# 获取命令行输入参数
def get_args():
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_path', type=str, required=True)
    parser.add_argument('--audio_format', type=str, default='wav')
    parser.add_argument('--sample_rate', type=str, default='8000')
    args = parser.parse_args()

    return args


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}" \
        .format(grant_type, client_secret, client_id)

    try:
        response = requests.post(url)
        response.raise_for_status()
    except Exception as e:
        print(response.text)
        raise Exception
    else:
        access_token = json.loads(response.text).get('access_token')
        return access_token


if __name__ == '__main__':
    try:
        args = get_args()

        # 获取access_token
        client_secret = args.client_secret
        client_id = args.client_id
        access_token = get_access_token(client_secret, client_id)

        # 准备数据
        data = prepare_data(args, access_token)

        uri = "wss://openapi.data-baker.com/asr/realtime"
        # 建立Websocket连接
        client = Client(data, uri)
        client.connect()
    except Exception as e:
        print(e)
