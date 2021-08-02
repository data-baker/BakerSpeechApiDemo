from flask import Flask, request
import json

app = Flask(__name__)


@app.route("/", methods=['GET', 'POST'])
def get_audio():
    data = str(json.loads(request.get_data()))
    with open('clong.log', 'w') as f:
        f.write(data)

    return "finished"
