#!/usr/bin/env python
# coding: utf-8

import requests
import json
import argparse


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}"\
        .format(grant_type, client_secret, client_id)

    try:
        response = requests.post(url)
        response.raise_for_status()
    except Exception as e:
        print(e)
        return
    else:
        access_token = json.loads(response.text).get('access_token')
    
    return access_token


# 获取识别后文本
def get_text(file, headers):
    url = "https://asr.data-baker.com/asr/api?"
    response = requests.post(url, data=file, headers=headers)
    code = json.loads(response.text).get("code")
    text = json.loads(response.text).get("text")
    if code != 20000:
        print(response.text)
    
    return text


# 获取命令行输入参数
def get_args():
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_path', type=str, required=True)
    parser.add_argument('--audio_format', type=str, default='wav')
    parser.add_argument('--sample_rate', type=str, default='16000')
    parser.add_argument('--add_pct', type=str, default='true')
    args = parser.parse_args()

    return args


if __name__ == '__main__':
    args = get_args()

    # 获取access_token
    client_secret = args.client_secret
    client_id = args.client_id
    access_token = get_access_token(client_secret, client_id)

    # 读取音频文件
    with open(args.file_path, 'rb') as f:
        file = f.read()

    # 填写Header信息
    audio_format = args.audio_format
    sample_rate = args.sample_rate
    add_pct = args.add_pct
    headers = {'access_token': access_token, 'audio_format': audio_format, 'sample_rate': sample_rate,
               'add_pct': add_pct}
    text = get_text(file, headers)
    print(text)




