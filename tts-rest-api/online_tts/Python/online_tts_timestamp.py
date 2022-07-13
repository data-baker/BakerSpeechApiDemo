#!/usr/bin/env python
#coding: utf-8
import requests
import json
import argparse
import base64


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(grant_type, client_secret, client_id)
    response = requests.post(url)
    access_token = json.loads(response.text).get('access_token')

    return access_token


# 获取转换后音频
def get_audio(data):
    url = "https://openapi.data-baker.com/tts?access_token={}&domain={}&language={}&voice_name={}&text={}&audiotype={}" \
          "&enable_subtitles={}&interval={}".format(data['access_domain'], data['domain'], data['language'],
           data['voice_name'], data['text'], data['audiotype'], data['enable_subtitles'], data['interval'])
    response = requests.post(url)
    content_type = response.headers['Content-Type']
    if 'audio' not in content_type:
        raise Exception(response.text)

    length = int.from_bytes(response.content[:4], byteorder='big', signed=False)
    json_data = json.loads((response.content[4: length + 4]).decode(encoding='gbk'))
    subtitles_list = json.loads((base64.b64decode(json_data.get('data')['interval-subtitles'])).decode()).get('subtitles_list')
    for subtitles in subtitles_list:
        print(subtitles)
    return response.content[length + 4:]

# 获取命令行输入参数
def get_args():
    text = '标贝科技'
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_save_path', type=str, required=True)
    parser.add_argument('--text', type=str, default=text)
    parser.add_argument('--audiotype', type=str, default='6')
    parser.add_argument('--domain', type=str, default='1')
    parser.add_argument('--language', type=str, default='zh')
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
        audiotype = args.audiotype
        domain = args.domain
        language = args.language
        voice_name = args.voice_name
        text = args.text
        data = {'access_domain': access_token, 'audiotype': audiotype, 'domain': domain, 'language': language,
                'voice_name': voice_name, 'text': text, 'enable_subtitles': "1", 'interval': '1'}
        content = get_audio(data)

        #保存音频文件
        with open('test.wav', 'wb') as audio:
            audio.write(content)
        print("task finished successfully")
    except Exception as e:
        print(e)



