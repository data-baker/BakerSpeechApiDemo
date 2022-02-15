import requests
import json
import argparse
import base64
import os
import time


class VoicePrint:

    # 获取声纹id
    def get_voiceprint_id(self, access_token):
        url = "https://openapi.data-baker.com/vpr/createid"
        headers = {'Content-Type': 'application/json'}
        data = {'access_token': access_token}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        register_id = json.loads(response.text).get("registerid")
        if json.loads(response.text).get('err_no') == 90000:
            return register_id
        else:
            raise Exception

    # 声纹注册
    def voiceprint_register(self, audio, access_token, name, format, registerId):
        url = 'https://openapi.data-baker.com/vpr/register'
        headers = {'Content-Type': 'application/json'}
        data = {'access_token': access_token,
                'format': format,
                'audio': audio,
                'registerId': registerId,
                'name': name,
                'scoreThreshold': 30.0}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        if json.loads(response.text).get('err_no') == 90000:
            return True
        else:
            print(response.text)
            return False

    # 查询声纹状态码
    def check_voiceprint_status(self, access_token, register_id):
        url = 'https://openapi.data-baker.com/vpr/status'
        headers = {'Content-Type': 'application/json'}
        data = {'access_token': access_token,
                'registerId': register_id}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        if json.loads(response.text).get('err_no') == 90000:
            if json.loads(response.text).get('status') == 3:
                return "register successfully"
            else:
                return "register failed"
        else:
            raise Exception

    # 声纹删除
    def remove_voiceprint(self, access_token, registerId):
        url = 'https://openapi.data-baker.com/vpr/delete'
        headers = {'Content-Type': 'application/json'}
        data = {'access_token': access_token,
                'registerId': registerId}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        if json.loads(response.text).get('err_no') == 90000:
            return "remove successfully"
        else:
            return "remove failed"

    # 声纹对比
    def verification_1VN(self, access_token, file):
        url = 'https://openapi.data-baker.com/vpr/search'
        headers = {'Content-Type': 'application/json'}

        data = {'access_token': access_token,
                'format': args.format,
                'audio': file,
                'listNum': 10,
                'scoreThreshold': 65.0}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        return response

    # 声纹认证
    def verification_1V1(self, access_token, file, matchId):
        url = 'https://openapi.data-baker.com/vpr/match'
        headers = {'Content-Type': 'application/json'}
        data = {'access_token': access_token,
                'format': 'pcm',
                'audio': file,
                'matchId': matchId,
                'scoreThreshold': 65.0}
        response = requests.post(url, data=json.dumps(data), headers=headers)
        return response


# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(
        grant_type, client_secret, client_id)
    response = requests.post(url)
    access_token = json.loads(response.text).get('access_token')

    return access_token


# 获取命令行输入参数
def get_args():
    parser = argparse.ArgumentParser(description='VR')
    parser.add_argument('-client_secret', type=str, required=True)
    parser.add_argument('-client_id', type=str, required=True)
    parser.add_argument('-file_path', type=str, required=True)
    parser.add_argument('-name', type=str, required=True)
    parser.add_argument('--format', type=str, required=False, default='pcm')
    parser.add_argument('--scoreThreshold', type=float, required=False, default=65.0)

    args = parser.parse_args()

    return args


if __name__ == '__main__':
    try:
        args = get_args()
        vp = VoicePrint()

        # 获取access_token
        client_secret = args.client_secret
        client_id = args.client_id
        file_path = args.file_path
        access_token = get_access_token(client_secret, client_id)

        # 创建声纹特征id
        registerId = vp.get_voiceprint_id(access_token)

        num_of_registered_audio = 0
        for root, dirs, files in os.walk(file_path):
            for file in files:
                with open(os.path.join(file_path, file), 'rb') as f:
                    audio = str(base64.b64encode(f.read()), encoding='utf-8')
                # 声纹注册
                if vp.voiceprint_register(audio, access_token, args.name, args.format, registerId):
                    num_of_registered_audio += 1
                if num_of_registered_audio >= 3:
                    print("register successfully")
                    break

        time.sleep(1)

        with open('data/1.wav', 'rb') as f:
            file = str(base64.b64encode(f.read()), encoding='utf-8')

        # 声纹对比（1 V N）
        response = vp.verification_1VN(access_token, file)
        print(response.text)

        # 声纹验证（1 V 1)
        # response = vp.verification_1V1(access_token, file, registerId)
        # print(response.text)

        # 查询声纹注册状态
        # print(vp.check_voiceprint_status(access_token, registerId))

        # 删除指定声纹
        # print(vp.remove_voiceprint(access_token, registerId))

        print('finished')

    except Exception as e:
        print(e)
