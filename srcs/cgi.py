#! /usr/local/bin/python3.9
import os
import sys
envs = ["CONTENT_LENGTH", "CONTENT_TYPE", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "REMOTE_ADDR", "REMOTE_HOST", "REQUEST_METHOD", "SCRIPT_NAME", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "SERVER_SOFTWARE", "QUERY_STRING"]

print("Status: 200")
print("Content-Type: text/html\n")
