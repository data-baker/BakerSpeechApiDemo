import argparse
import json
import base64
from threading import Thread
import requests
import websocket
import wave


#websocket客户端
class Client:
    def __init__(self, data, uri):
        self.data = data
        self.uri = uri
        self.audio_data = b""

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
        print("sending..")
        def run(*args):
            ws.send(self.data)

        Thread(target=run).start()

    # 接收消息
    def on_message(self, ws, message):
        message = json.loads(message)
        code = message["code"]
        if 'subtitles_list' in message["data"]:
            print(message["data"]['subtitles_list'])
        if code != 90000:
            # 打印接口错误
            print(message)
        else:
            self.audio_data += base64.b64decode(bytes(message["data"]["audio_data"], encoding='utf-8'))
            if message.get("data")["end_flag"] == 1:
                with wave.open('test.wav', 'wb') as wavfile:
                    wavfile.setparams((1, 2, 16000, 0, 'NONE', 'NONE'))
                    wavfile.writeframes(self.audio_data)
                ws.close()
                print("task finished successfully")

    # 打印错误
    def on_error(self, ws, error):
        print("error: ", str(error))

    # 关闭连接
    def on_close(ws):
        print("client closed.")


# 准备数据
def prepare_data(args, access_token):

    # 填写Header信息
    audiotype= args.audiotype
    voice_name = args.voice_name

    text = args.text
    # 单次调用最长300个汉字
    if len(text) > 300:
        raise Exception("Text is too long. The maximum length of chinese character is 300")
    text_bytes = text.encode(encoding='UTF-8')
    text = str(base64.b64encode(text_bytes), encoding='UTF-8')
    tts_params = {"language": "ZH", "voice_name": voice_name, "audiotype": audiotype, "domain": "1", "text": text, 'interval': '1', 'enable_subtitles': '1'}

    data = {"access_token": access_token, "version": "1.0", "tts_params": tts_params}
    data = json.dumps(data)

    return data


# 获取命令行输入参数
def get_args():
    text = '标贝科技'
    parser = argparse.ArgumentParser(description='tts')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_save_path', type=str, required=True)
    parser.add_argument('--text', type=str, default=text)
    parser.add_argument('--audiotype', type=str, default='4')
    parser.add_argument('--voice_name', type=str, default='beixi')
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

        uri = "wss://openapi.data-baker.com/tts/wsapi"
        # 建立Websocket连接
        client = Client(data, uri)
        client.connect()

        print('end')
    except Exception as e:
        print(e)
