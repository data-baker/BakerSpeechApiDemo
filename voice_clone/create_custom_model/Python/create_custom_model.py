import requests
import json
import argparse


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(grant_type, client_secret, client_id)
    response = requests.post(url)
    access_token = json.loads(response.text).get('access_token')

    return access_token


# 创建模型
def create_model(data, files):
    url = "https://openapi.data-baker.com/gramophone/v1/submit"
    response = requests.post(url, data=data, files=files)
    code = json.loads(response.text).get("code")
    if code != "20000":
        raise Exception(response.text)
    else:
        model = json.loads(response.text).get("data")["modelId"]
        print(model)


# 获取命令行输入参数
def get_args():
    parser = argparse.ArgumentParser(description='ASR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-notifyUrl', type=str, required=True)
    parser.add_argument('--mobile', type=str)

    args = parser.parse_args()

    return args


if __name__ == '__main__':
    try:
        args = get_args()

        # 获取access_token
        client_secret = args.client_secret
        client_id = args.client_id
        access_token = get_access_token(client_secret, client_id)

        # 填写文件路径
        files = [
            ('originFiles', (
            '101.wav', open('101.wav', 'rb'),
            'audio/wav')),
            ('originFiles', (
            '201.wav', open('201.wav', 'rb'),
            'audio/wav')),
            ('originFiles', (
            '301.wav', open('301.wav', 'rb'),
            'audio/wav')),
            ('originFiles', (
            '401.wav', open('401.wav', 'rb'),
            'audio/wav'))
        ]
        # 读取参数
        notifyUrl = args.notifyUrl
        mobile = args.mobile
        data = {'access_token': access_token, 'notifyUrl': notifyUrl, 'mobile': mobile}
        create_model(data, files)
        print("task finished successfully")
    except Exception as e:
        print(e)