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


# 创建识别任务，发送识别文件
def send_text(data):
    url = "https://openapi.data-baker.com/asynctts/synthesis/work"
    headers = {'content-type': 'application/json'}
    response = requests.post(url, data=data, headers=headers)
    code = json.loads(response.text).get("code")
    if code != 0:
        raise Exception(response.text)


# 获取命令行输入参数
def get_args():
    text = '欢迎使用标贝开发平台。'
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-notify_url', type=str, required=True)
    parser.add_argument('--text', type=str, default=text)
    parser.add_argument('--voice_name', type=str, default='Lingling')
    parser.add_argument('--audiotype', type=str, default='6')
    args = parser.parse_args()

    return args


def main(args):
    # 获取access_token
    client_secret = args.client_secret
    client_id = args.client_id
    access_token = get_access_token(client_secret, client_id)

    # 新建任务
    voice_name = args.voice_name
    text = args.text
    # 回调地址
    notify_url = args.notify_url
    headers = {'access_token': access_token, 'text': text, 'voiceName': voice_name, 'notifyUrl': notify_url}
    send_text(json.dumps(headers))
    print("send text successfully!")

if __name__ == '__main__':
    try:
        args = get_args()
        main(args)
    except Exception as e:
        print(e)