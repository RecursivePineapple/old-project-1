
from flask import Flask, jsonify, request

app = Flask(__name__)

@app.route("/network/ip")
def get_address():
    return jsonify({"address": request.remote_addr}), 200

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=8082)
