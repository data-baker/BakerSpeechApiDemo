#!/usr/bin/env python
# coding: utf-8
import requests
import json
import argparse
import base64


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(
        grant_type, client_secret, client_id)
    response = requests.post(url)
    access_token = json.loads(response.text).get('access_token')

    return access_token


# 获取转换后音频
def get_audio(data):
    url = "https://openapi.data-baker.com/tts"
    response = requests.post(url, json=data)
    content_type = response.headers['Content-Type']
    if 'audio' not in content_type:
        raise Exception(response.text)

    length = int.from_bytes(response.content[:4], byteorder='big', signed=False)
    json_data = json.loads((response.content[4: length + 4]).decode(encoding='gbk'))
    segments_list = json.loads((base64.b64decode(json_data.get('timestamp'))).decode()).get('segments')
    for segments in segments_list:
        print(segments)
    return response.content[length + 4:]


# 获取命令行输入参数
def get_args():
    text = '标贝科技'
    parser = argparse.ArgumentParser(description='TTS')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('--text', type=str, default=text)
    parser.add_argument('--voice_name', type=str, default='Jiaojiao')
    args = parser.parse_args()

    return args


if __name__ == '__main__':
    try:
        args = get_args()

        # 获取access_token
        client_secret = args.client_secret
        client_id = args.client_id
        access_token = get_access_token(client_secret, client_id)

        # 读取参数
        voice_name = args.voice_name
        text = args.text

        params = {
            "version": "2.1",
            "access_token": access_token,
            "tts_params": {
                "text": text,
                "domain": "1",
                "voice_name": voice_name,
                "language": "zh",
                "timestamp": "phone",
                "audio_fmt": "WAV"
            },
        }
        content = get_audio(params)

        # 保存音频文件
        with open('test.wav', 'wb') as audio:
            audio.write(content)
        print("task finished successfully")
    except Exception as e:
        print(e)


"""
python online_tts_timestamp.py -client_secret=your_client_secret -client_id=your_client_id --text=今天天气不错哦
"""
