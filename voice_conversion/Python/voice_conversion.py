import argparse
import json
import wave
import requests
import websocket
from threading import Thread


#websocket客户端
class Client:
    def __init__(self, data, uri, save_path):
        self.data = data
        self.uri = uri
        self.converted_data = b""
        self.save_path = save_path

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
            for message in self.data:
                ws.send(message, websocket.ABNF.OPCODE_BINARY)

        Thread(target=run).start()

    # 接收消息
    def on_message(self, ws, message):
        length = int.from_bytes(message[:4], byteorder='big', signed=False)
        json_data = json.loads((message[4: length + 4]).decode())
        if json_data['errcode'] != 0:
            print(json_data)
            raise Exception
        self.converted_data += message[4 + length:]
        if json_data['lastpkg']:
            with wave.open(self.save_path, 'wb') as wavfile:
                wavfile.setparams((1, 2, 16000, 0, 'NONE', 'NONE'))
                wavfile.writeframes(self.converted_data)
                ws.close()
                print("task finished successfully")
        code = json.loads(message).get("errcode")
        print(str(json.loads(message)))
        if code != 0:
            # 打印接口错误
            print(message)

    # 打印错误
    def on_error(slef, ws, error):
        print("error: ", str(error))

    # 关闭连接
    def on_close(ws):
        print("client closed.")


# 准备数据
def prepare_data(args, access_token):

    # 填写Header信息
    voice_name = args.voice_name
    with open(args.file_path, 'rb') as f:
        file = f.read()
    data = []

    for i in range(0, len(file), 32000):
        if i + 32000 > len(file):
            tts_params = {"access_token": access_token, "voice_name": voice_name, 'enable_vad': True, 'align_input': True, "lastpkg": True}
        else:
            tts_params = {"access_token": access_token, "voice_name": voice_name, 'enable_vad': True, 'align_input': True, "lastpkg": False}
        json_data = json.dumps(tts_params)
        json_data_bi = json_data.encode()
        length = len(json_data)
        head_data = length.to_bytes(4, byteorder='big')

        if i + 32000 > len(file):
            data.append(head_data + json_data_bi + file[i:])
        else:
            data.append(head_data + json_data_bi + file[i: i + 32000])

    return data


# 获取命令行输入参数
def get_args():
    text = "今天天气不错哦！"
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_path', type=str, required=True)
    parser.add_argument('-file_save_path', type=str, required=True)
    parser.add_argument('--voice_name', type=str, default='Vc_baklong')
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
        access_token = access_token = get_access_token(client_secret, client_id)

        # 准备数据
        data = prepare_data(args, access_token)

        uri = "wss://openapi.data-baker.com/ws/voice_conversion"
        # 建立Websocket连接
        client = Client(data, uri, args.file_save_path)
        client.connect()
    except Exception as e:
        print(e)