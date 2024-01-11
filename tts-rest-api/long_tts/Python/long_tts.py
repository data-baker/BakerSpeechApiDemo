import argparse
import requests
import json
import time

# 获取access_token用于鉴权
def get_access_token(client_secret, client_id):
    grant_type = "client_credentials"
    url = "https://openapi.data-baker.com/oauth/2.0/token?grant_type={}&client_secret={}&client_id={}".format(
        grant_type, client_secret, client_id
    )
    response = requests.post(url)
    access_token = json.loads(response.text).get("access_token")

    return access_token

# 创建任务，发送文字
def send_text(data):
    url = "https://openapi.data-baker.com/asynctts/synthesis/work"
    headers = {"content-type": "application/json"}
    response = requests.post(url, data=data, headers=headers)
    code = json.loads(response.text)["err_no"]
    if code != 0:
        raise Exception(response.text)
    else:
        return json.loads(response.text)["result"]["work_id"]

# 查询合成结果
def get_audio(params):
    url = "https://openapi.data-baker.com/asynctts/synthesis/query"
    response = requests.get(url, params=params)
    resposne_json = response.json()
    if resposne_json["err_no"] == 0:
        audio_url = resposne_json["result"]["audio_url"]
        return audio_url
    else:
        print(resposne_json)

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

def main(args):
    # 获取access_token
    client_secret = args.client_secret
    client_id = args.client_id
    access_token = get_access_token(client_secret, client_id)

    # 新建合成任务

    params = {
        "version": "2.1",
        "access_token": access_token,
        "tts_params": {
            "text": args.text,
            "voice_name": args.voice_name,
            "language": "zh",
        },
    }
    print(params)
    workId = send_text(json.dumps(params))
    print(workId)
    print("send text successfully!", "  ", workId)

    # 查询合成结果
    query_params = {"client_id": client_id, "access_token": access_token, "work_id": workId, "version": "2.1"}
    while True:
        audio_url = get_audio(query_params)
        if audio_url:
            break
        else:
            time.sleep(2)
            print("task is processing")

    audio_data = requests.get(audio_url)
    with open("test.mp3", "wb") as f:
        f.write(audio_data.content)
    print("task finished")

if __name__ == "__main__":
    args = get_args()
    main(args)

"""
python long_tts.py -client_secret=your_client_secret -client_id=your_client_id --text=我期待的不是雪而是有你的冬天
"""