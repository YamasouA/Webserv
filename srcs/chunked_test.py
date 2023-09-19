import traceback
import http.client
import os

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}
LOCATION_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive', 'Location': 'tmp'}
CLOSE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'close'}

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

def create_chunked_data(chunked_data_path, err):
	data = ""
	total = 0
	with open(chunked_data_path, 'r') as f:
		while True:
			line = f.readline()
			if not line:
				break
			total += len(line)
			data += format(len(line), 'x')
			data += "\r\n"
			#data += line.replace('\n', '\r\n')
			data += line + "\r\n"
	if err == 0:
		data += "0\r\n\r\n"
	return bytes(data, encoding='utf-8'), total

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
			print(data_size, ": ", res_header['Content-Length'])
			if res_header['Content-Length'] != str(data_size):
				return False

		elif header_field == 'Location':
			continue

		# その他のヘッダーは値が一致する必要ある
		elif res_header[header_field] != expect_header[header_field]:
			print(res_header[header_field])
			return False

	return True


def get_body(status_code, path):
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

def response_test(path, expected_status, expected_headers, file_path, prefix, err=0):
	conn = http.client.HTTPConnection("localhost:8000")
	conn.putrequest("POST", path)
	
	# ヘッダーに Transfer-Encoding: chunked を追加
	conn.putheader('Transfer-Encoding', 'chunked')
	conn.endheaders()

	data_chunked, _ = create_chunked_data(file_path, err)
	expected_body = get_body(expected_status, file_path)
	content_length = len(expected_body)
	if expected_status == 200:
		body2 = get_body(expected_status, (prefix + path).replace('//', '/'))
		expected_body += body2
		content_length += len(body2)
	
	# データをチャンクとして送信
	conn.send(data_chunked)
	
	res = conn.getresponse()
	data = res.read()
	assert res.status == expected_status, \
		"Status_code Error" + error_text(expected_status, res.status)
	
	assert header_checker(expected_headers, res.headers, content_length),\
		"Header Error" + error_text(expected_headers, res.headers)

	assert data.decode('utf-8') == data.decode('utf-8'),\
		"Body Error" + error_text(data, data.decode('utf-8'))

	print(path + " test done")

def clean_up(upload_path, target_file):
	if os.path.exists(upload_path + target_file):
		os.remove(upload_path + target_file)

def CHUNKED_test():
	upload_path = "upload/"
	clean_up(upload_path, "post.html")
	# 通常のPOST
	response_test("/post.html", 201, LOCATION_HEADERS, "index.html", upload_path)
	response_test("/post.html", 200, SIMPLE_HEADERS, "index.html", upload_path)
	# ディレクトリを指定してPOST
	response_test("/", 201, LOCATION_HEADERS, "index.html", upload_path)
	# 正しくないデータフォーマット
	response_test("/index.html", 408, CLOSE_HEADERS, "index.html", upload_path, err=1)


if __name__ == "__main__":
	CHUNKED_test()
