import requests
import traceback
import os

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}

def error_text(str1, str2):
	return "expect:\n{}\nbut:{}".format(str1, str2)

def create_path(path):
	return SCHEME + "://" + HOST_NAME + path

# レスポンスのヘッダーが正しいかを確認する
def header_checker(expect_header, res_header, expect_body):
	if len(expect_header) != len(res_header):
		return False

	for header_field in expect_header:
		# 'Date'は存在だけ確認できればいい
		if header_field == 'Date':
			continue
		
		# 'Content-Length'はgetしたファイルのサイズと比較する
		if header_field == 'Content-Length':
			if res_header['Content-Length'] != str(len(expect_body)):
				return False

		# その他のヘッダーは値が一致する必要ある
		elif res_header[header_field] != expect_header[header_field]:
			print(res_header[header_field])
			return False

	return True

def response_test(url, expected_status, expected_headers, file_path):
	res = requests.delete(url)

	assert res.status_code == expected_status,\
		"Status_code Error" + error_text(expected_status, res.status_code)


	is_exist = os.path.exists(file_path)

	assert header_checker(expected_headers, res.headers, ""),\
		"Header Error" + error_text(expected_headers, res.headers)

	# fileが見つかってないから
	assert  not is_exist,\
		"Body Error"

	print(url + " test done")

def set_up(file_list):
	for filename in file_list:
		with open(filename, "w") as f:
			f.write("HELLO WORLD")

def DELETE_test():
	file_list = ["post.html"]
	set_up(file_list)
	try:
		# 正常系
		response_test(create_path("/post.html"), 200, SIMPLE_HEADERS, "post.html")
		# 存在しないファイル
		response_test(create_path("/wwwwwwwwwwwwww.html"), 404, SIMPLE_HEADERS, "wwwwwwwwwwwww.html")
		# 
		#response_test(create_path("/.html"), 204, SIMPLE_HEADERS, "wwwwwwwwwwwww.html")

	except:
		traceback.print_exc()
		


if __name__ == "__main__":
	DELETE_test()
