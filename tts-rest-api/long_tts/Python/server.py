from flask import Flask, request
import json
import wget

app = Flask(__name__)


@app.route("/", methods=['GET', 'POST'])
def hello_world():
    url = json.loads(request.get_data())['audioUrl']
    wget.download(url)
    print("task finished successfully")

    return "finished"
