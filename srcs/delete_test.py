import requests
import traceback
import os

SCHEME = "http"
HOST_NAME = "localhost:8000"

NOT_FOUND_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}
SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Connection': 'keep-alive'}

m = {
    200: "OK",
    201: "Created",
    202: "Accepted",
    203: "",
    204: "No Content",
    205: "",
    206: "Partial Content",
    301: "Moved Permanently",
    302: "Moved Temporarily",
    303: "See Other",
    304: "Not Modified",
    307: "Temporary Redirect",
    308: "Permanent Redirect",

    400: "Bad Request",
    401: "Unauthorized",
    402: "Payment Required",
    403: "Forbidden",
    404: "Not Found",
    405: "Not Allowed",
    406: "Not Acceptable",
    408: "Request Time-out",
    409: "Conflict",
    410: "Gone",
    411: "Length Required",
    412: "Precondition Failed",
    413: "Request Entity Too Large" ,
    414: "Request-URI Too Large",
    415: "Unsupported Media Type",
    416: "Requested Range Not Satisfiable",
    421: "Misdirected Request",
    429: "Too Many Requests",

    500: "Internal Server Error",
    501: "Not Implemented",
    502: "Bad Gateway",
    503: "Service Temporarily Unavailable",
    504: "Gateway Time-out",
    505: "HTTP Version Not Supported",
    507: "Insufficient Storage",
}

def error_text(str1, str2):
	return "expect:\n{}\nbut:{}".format(str1, str2)

def create_path(path):
	return SCHEME + "://" + HOST_NAME + path

def get_body_from_status(status_code):
	# それぞれのstatus_codeにデフォルトの内容が存在する
	if status_code != 200 and status_code != 204:
		return "<html>\r\n<head><title>" + str(status_code) + " " + m[status_code]\
			+ "</title></head>\r\n<body>\r\n<center><h1>"\
			+ str(status_code) + " " + m[status_code]\
			+ "</h1></center>\r\n<hr><center>WebServe</center>\r\n</body>\r\n</html>"
	else:
		return ""
# レスポンスのヘッダーが正しいかを確認する
def header_checker(expect_header, res_header, expect_body):
	if len(expect_header) != len(res_header):
		print("len diff")
		return False

	for header_field in expect_header:
		print(header_field)
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
	expect_body = get_body_from_status(expected_status)

	assert header_checker(expected_headers, res.headers, expect_body),\
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
	file_list = ["delete.html"]
	set_up(file_list)
	try:
		# 正常系
		response_test(create_path("/delete.html"), 204, SIMPLE_HEADERS, "delete.html")
		# 存在しないファイル
		response_test(create_path("/wwwwwwwwwwwwww.html"), 404, NOT_FOUND_HEADERS, "")

	except:
		traceback.print_exc()
		


if __name__ == "__main__":
	DELETE_test()
