 #!/usr/bin/env python
# coding: utf-8

import argparse
import requests
import json


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(
        grant_type, client_secret, client_id)
    response = requests.post(url)
    access_token = json.loads(response.text).get('access_token')

    return access_token


# 查询识别结果
def get_text(headers):
    url = "https://asr.data-baker.com/asr/taskresult"
    response = requests.post(url, headers=headers)
    text = json.loads(response.text).get("text")
    code = json.loads(response.text).get("code")
    if code != 20000:
        raise Exception(response.text)

    return text


# 创建识别任务，发送识别文件
def create_task(file, headers):
    url = "https://asr.data-baker.com/asr/starttask?"
    response = requests.post(url, data=file, headers=headers)
    task_id = json.loads(response.text).get("taskid")
    code = json.loads(response.text).get("code")
    if code != 20000:
        raise Exception(response.text)

    return task_id


# 获取命令行输入参数
def get_args():
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_path', type=str, required=True)
    parser.add_argument('--audio_format', type=str, default='wav')
    parser.add_argument('--sample_rate', type=str, default='16000')
    args = parser.parse_args()

    return args


if __name__ == '__main__':
    try:
        args = get_args()

        # 获取access_token
        client_secret = args.client_secret
        client_id = args.client_id
        access_token = get_access_token(client_secret, client_id)

        # 新建任务
        file = open(args.file_path, 'rb')
        audio_format = args.audio_format
        sample_rate = args.sample_rate
        callback_url = 'http://82.157.6.4:8000/'
        headers = {'access_token': access_token, 'audio_format': audio_format, 'sample_rate': sample_rate, 'callback_url': callback_url}
        task_id = create_task(file, headers)
        print("create task successfully!")

    except Exception as e:
        print(e)