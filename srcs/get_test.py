import requests
import traceback

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}

def error_text(str1, str2):
	return "expect:\n{}\nbut:{}".format(str1, str2)

def create_path(path):
	return SCHEME + "://" + HOST_NAME + path

# レスポンスのヘッダーが正しいかを確認する
def header_checker(expect_header, res_header, expect_body):
	print(expect_header)
	print(res_header)
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

def response_test(url, expected_status, expected_headers, body_path):
	res = requests.get(url)

	assert res.status_code == expected_status,\
		"Status_code Error" + error_text(expected_status, res.status_code)

	try:
		with open(body_path, 'r', encoding=res.encoding) as f:
			data = f.read()
	except FileNotFoundError:
		data = ""

	print(len(data))
	assert header_checker(expected_headers, res.headers, data),\
		"Header Error" + error_text(expected_headers, res.headers)

	assert res.text == data,\
		"Body Error" + error_text(data, res.text)

	print(url + " test done")

def GET_test():
	try:
		# response_test(uri, expected status_code, expected haders, return file's path)
		response_test(create_path("/post.html"), 200, SIMPLE_HEADERS, "post.html")
		response_test(create_path("/index.html"), 200, SIMPLE_HEADERS, "index.html")
		response_test(create_path("/"), 200, SIMPLE_HEADERS, "index.html")
		#response_test(create_path("/wwwwwwwwwwwwww.html"), 404, SIMPLE_HEADERS, "wwwwwwwwwwwww.html")
	except:
		traceback.print_exc()
		


if __name__ == "__main__":
	GET_test()