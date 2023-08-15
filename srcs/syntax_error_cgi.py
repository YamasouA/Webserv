#! /usr/bin/env python3
import os
import sys
envs = ["HTTP_CONTENT_LENGTH", "CONTENT_TYPE", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "REMOTE_ADDR", "REMOTE_HOST", "REQUEST_METHOD", "REQUEST_NAME", "SCRIPT_NAME", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "SERVER_SOFTWARE"]

print("Content-Type: text/html")
print("Status : 200\n")

for env in envs:
	if os.getenv(env):
		val = os.getenv(env)
	else:
		val = ""
	print(env + "=" + val) 
