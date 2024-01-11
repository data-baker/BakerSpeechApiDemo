import requests
import json
import argparse

# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(
        grant_type, client_secret, client_id
    )
    response = requests.post(url)
    access_token = json.loads(response.text).get("access_token")

    return access_token

# 获取转换后音频
def get_audio(data):
    url = "https://openapi.data-baker.com/tts"
    params = {
        "version": "2.1",
        "access_token": data["access_token"],
        "tts_params": {
            "text": data["text"],
            "domain": "1",
            "voice_name": data["voice_name"],
            "language": "zh",
        },
    }
    response = requests.post(url, json=params)
    content_type = response.headers["Content-Type"]
    if "audio" not in content_type:
        raise Exception(response.text)
    return response.content

# 获取命令行输入参数
def get_args():
    text = "欢迎使用标贝开发平台。"
    parser = argparse.ArgumentParser(description="ASR")
    parser.add_argument("-client_secret", type=str, required=True)
    parser.add_argument("-client_id", type=str, required=True)
    parser.add_argument("--text", type=str, default=text)
    parser.add_argument("--voice_name", type=str, default="Jiaojiao")
    args = parser.parse_args()

    return args

if __name__ == "__main__":
    args = get_args()

    # 获取access_token
    client_secret = args.client_secret
    client_id = args.client_id
    access_token = get_access_token(client_secret, client_id)

    # 读取参数
    data = {"access_token": access_token, "voice_name": args.voice_name, "text": args.text}
    content = get_audio(data)

    # 保存音频文件
    with open("test.wav", "wb") as audio:
        audio.write(content)
    print("task finished successfully")

"""
python online_tts.py -client_secret=your_client_secret -client_id=your_client_id -file_save_path=test.wav --text=今天天气不错哦
"""
