#! /usr/bin/env python3
import os
import sys
envs = ["CONTENT_LENGTH", "CONTENT_TYPE", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "REMOTE_ADDR", "REMOTE_HOST", "REQUEST_METHOD", "SCRIPT_NAME", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "SERVER_SOFTWARE", "QUERY_STRING"]

print("Content-Type: text/html\n")

# Content-Length ヘッダーからデータ長を取得
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
# 標準入力からデータを読み取る
body = sys.stdin.read(content_length)
print(body, end="")
