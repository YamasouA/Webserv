#! /usr/bin/env python3
import os
import sys
envs = ["HTTP_CONTENT_LENGTH", "CONTENT_TYPE", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "REMOTE_ADDR", "REMOTE_HOST", "REQUEST_METHOD", "REQUEST_NAME", "SCRIPT_NAME", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "SERVER_SOFTWARE"]

print("Content-Type: text/html\n")

for env in envs:
	if os.getenv(env):
		val = os.getenv(env)
	else:
		val = ""
	print(env + "=" + val) 

# Content-Length ヘッダーからデータ長を取得
content_length = int(os.environ.get("HTTP_CONTENT_LENGTH", 0))
print(content_length)
# 標準入力からデータを読み取る
body = sys.stdin.read(content_length)
print("body")
print(body)
print("end cgi")
