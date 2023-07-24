import requests
import traceback

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}
REQUEST_BODY = "HELLO WORLD"

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

def response_test(url, expected_status, expected_headers, request_body, file_path):
	try:
		with open(file_path, 'r', encoding='utf-8') as f:
			bef_data = f.read()
	except FileNotFoundError:
		bef_data = ""
	
	res = requests.post(url, request_body)
	expect_data = bef_data + request_body

	assert res.status_code == expected_status,\
		"Status_code Error" + error_text(expected_status, res.status_code)

	try:
		with open(file_path, 'r', encoding='utf-8') as f:
			aft_data = f.read()
	except FileNotFoundError:
		aft_data = ""

	assert header_checker(expected_headers, res.headers, expect_data),\
		"Header Error" + error_text(expected_headers, res.headers)

	assert  expect_data == aft_data,\
		"Body Error" + error_text(expect_data, aft_data)

	print(url + " test done")

def POST_test():
	try:
		response_test(create_path("/post.html"), 200, SIMPLE_HEADERS, REQUEST_BODY, "post.html")
		#response_test(create_path("/wwwwwwwwwwwwww.html"), 404, SIMPLE_HEADERS, "wwwwwwwwwwwww.html")
	except:
		traceback.print_exc()
		


if __name__ == "__main__":
	POST_test()
