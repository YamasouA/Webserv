import requests
import traceback

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}

CLOSE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'close'}

ALLOW_HEADERS = {'Allow': 'POST ', 'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}

UNSUPPORT_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Length':'tmp', 'Connection': 'keep-alive', 'Location': 'tmp'}

NO_CONTENT_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Connection': 'keep-alive'}
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
	if status_code != 200 and status_code != 204:
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
def header_checker(expect_header, res_header, data_size):
	if len(expect_header) != len(res_header):
		return False

	for header_field in expect_header:
		print(header_field)
		# 'Date'は存在だけ確認できればいい
		if header_field == 'Date':
			continue
		
		# 'Content-Length'はgetしたファイルのサイズと比較する
		if header_field == 'Content-Length':
			print(data_size)
			if res_header['Content-Length'] != str(data_size):
				return False

		# その他のヘッダーは値が一致する必要ある
		elif res_header[header_field] != expect_header[header_field]:
			return False

	return True

def response_test(url, expected_status_list, expected_headers, body_path):
	res = requests.get(url)
	if len(res.history) >= 1:
		# 最終的なステータスはhistoryに入らない
		assert len(res.history) == len(expected_status_list) - 1,\
			"redirect num Error" + error_text(expected_status_list, res.history)
		for hi, exp_st in zip(res.history, expected_status_list):
			if hi.status_code != exp_st:
				assert len(res.history) == len(expected_status_list),\
					"redirect num Error" + error_text(expected_status_list, res.history)
	expected_status = expected_status_list[-1]
	assert res.status_code == expected_status,\
		"Status_code Error" + error_text(expected_status, res.status_code)

	data = get_body_from_status(expected_status, body_path)
	assert header_checker(expected_headers, res.headers, len(data)),\
		"Header Error" + error_text(expected_headers, res.headers)

	assert res.text == data,\
		"Body Error" + error_text(data, res.text)

	print(url + " test done")

def GET_test():
	try:
		# response_test(uri, expected status_code, expected haders, return file's path)
		# 正常なテスト
		response_test(create_path("/index.html"), [200], SIMPLE_HEADERS, "index.html")
		# autoindex
		response_test(create_path("/"), [200], SIMPLE_HEADERS, "index.html")
		# ファイルはあるが中身がない
		response_test(create_path("/no_content.html"), [204], NO_CONTENT_HEADERS, "no_content.html")
		# 存在しないファイル
		response_test(create_path("/wwwwwwwwwwwwww.html"), [404], SIMPLE_HEADERS, "wwwwwwwwwwwww.html")
		# GET禁止
		response_test(create_path("/GET_DENIED"), [405], ALLOW_HEADERS, "wwwwwwwwwwwww.html")
		# redirect
		response_test(create_path("/redirect/hoge.txt"), [301, 200], SIMPLE_HEADERS, "index.html")
		# CGI
		response_test(create_path("/CGI/cgi.py"), [200], SIMPLE_HEADERS, "")
		# CGIタイムアウト
		response_test(create_path("/CGI/cgi_time_out.py"), [504], SIMPLE_HEADERS, "")
		# CGI設定されていない(cgiを実行するのではなく、staticHandlerに入る)
		response_test(create_path("/CGI_DENIED/cgi.py"), [200], SIMPLE_HEADERS, "./CGI_DENIED/cgi.py")
		# CGI自体がエラー(ステータスコードは幾つになるかわからん)
		response_test(create_path("/CGI/syntax_error_cgi.py"), [502], SIMPLE_HEADERS, "")
		# URIが長すぎる(TIME_OUT_HEADERだけど、connection Closeで会ってもらいたい)
		response_test(create_path("/" + 'a'*100000), [414], CLOSE_HEADERS, "")

		print("========= test done!!!!! ==========")
	except:
		traceback.print_exc()
		

if __name__ == "__main__":
	GET_test()

