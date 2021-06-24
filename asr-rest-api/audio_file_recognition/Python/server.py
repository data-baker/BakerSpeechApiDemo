from flask import Flask, request
import json


app = Flask(__name__)


@app.route("/", methods=['GET', 'POST'])
def hello_world():
    with open('test.txt', 'w') as f:
        text = json.loads(request.get_data())['text']['left_result']
        f.write(text)
    return "finished"

