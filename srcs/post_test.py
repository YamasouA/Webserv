import requests
import traceback
import os

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}
REDIRECT_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive', 'Location': 'tmp'}
REQUEST_BODY = "HELLO WORLD"

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

def get_body_from_status(status_code, path):
	# それぞれのstatus_codeにデフォルトの内容が存在する
	if status_code != 200 and status_code != 201:
		return "<html>\r\n<head><title>" + str(status_code) + " " + m[status_code]\
			+ "</title></head>\r\n<body>\r\n<center><h1>"\
			+ str(status_code) + " " + m[status_code]\
			+ "</h1></center>\r\n<hr><center>WebServe</center>\r\n</body>\r\n</html>"
	else:
		try:
			with open(path, 'r') as f:
				return f.read()
		except FileNotFoundError:
			return ""
	return ""
# レスポンスのヘッダーが正しいかを確認する
def header_checker(status_code, expect_header, res_header, file_path, expect_body):
	root = os.getcwd() + '/'

	for header_field in expect_header:
		print(header_field)
		# 'Date'は存在だけ確認できればいい
		if header_field.lower() == 'date':
			continue
		elif header_field.lower() == 'location' and status_code != 201:
			continue
		# 'Content-Length'はgetしたファイルのサイズと比較する
		elif header_field.lower() == 'content-length':
			if res_header[header_field] != str(len(expect_body)):
				print(res_header['Content-Length'], " ", len(expect_body))
				return False
		elif header_field.lower() == 'location':
			if res_header[header_field] != root + file_path:
				return False

		# その他のヘッダーは値が一致する必要ある
		elif res_header[header_field] != expect_header[header_field]:
			return False

	return True

def response_test(url, expected_status, expected_headers, request_body, file_path):
	bef_data = get_body_from_status(expected_status, file_path)
	
	res = requests.post(url, request_body)
	if res.status_code == 200 or res.status_code == 201:
		expect_data = bef_data + request_body
	else:
		expect_data = bef_data

	assert res.status_code == expected_status,\
		"Status_code Error" + error_text(expected_status, res.status_code)

	assert header_checker(res.status_code, expected_headers, res.headers, file_path, expect_data),\
		"Header Error" + error_text(expected_headers, res.headers)

	if res.status_code == 200 or res.status_code == 201:
		assert  expect_data == res.text,\
			"Body Error" + error_text(expect_data, aft_data)

	print(url + " test done")

def clean_up(upload_path, target_file):
	if os.path.exists(upload_path + target_file):
		os.remove(upload_path + target_file)

def POST_test():
	upload_path = "upload/"
	clean_up(upload_path, "post.html")
	try:
		# 新規作成
		response_test(create_path("/post.html"), 201, SIMPLE_HEADERS, REQUEST_BODY, upload_path+"post.html")
		# 追記
		response_test(create_path("/post.html"), 200, SIMPLE_HEADERS, REQUEST_BODY, upload_path+"post.html")
		# POSTを禁止している場所
		response_test(create_path("/POST_DENIED/post.html"), 405, SIMPLE_HEADERS, REQUEST_BODY, upload_path+"post.html")
		
		# ディレクトリを指定して作成(確認の仕方が思いついてない)
		# POST配下に何もできないけど200が返ってくる
		#response_test(create_path("/POST"), 201, SIMPLE_HEADERS, REQUEST_BODY, "")

		# CGI
		response_test(create_path("/POST/cgi_post.py"), 200, SIMPLE_HEADERS, REQUEST_BODY, "")
		# CGI設定されていない
		response_test(create_path("/CGI_DENIED/cgi_post.py"), 405, SIMPLE_HEADERS, REQUEST_BODY, "")
		# CGI自体がエラー
		response_test(create_path("/CGI/syntax_errro.py"), 502, SIMPLE_HEADERS, REQUEST_BODY, "")

	except:
		traceback.print_exc()
		


if __name__ == "__main__":
	POST_test()
