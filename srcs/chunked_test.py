import traceback
import http.client

SCHEME = "http"
HOST_NAME = "localhost:8000"

SIMPLE_HEADERS = {'Server': 'WebServe', 'Date': 'hoge', 'Content-Type': 'text/html', 'Content-Length':'tmp', 'Connection': 'keep-alive'}

def error_text(str1, str2):
	return "expect:\n{}\nbut:{}".format(str1, str2)

def create_path(path):
	return SCHEME + "://" + HOST_NAME + path

def create_chunked_data(chunked_data_path, err):
	data = ""
	with open(chunked_data_path, 'r') as f:
		while True:
			line = f.readline()
			if not line:
				break
			data += str(len(line))
			data += "\r\n"
			data += line
	if err == 0:
		data += "\r\n0\r\n\r\n"
	print(data)
	return bytes(data, encoding='utf-8')

def header_checker(expect_header, res_header, data_size):
	if len(expect_header) != len(res_header):
		return False

	for header_field in expect_header:
		# 'Date'は存在だけ確認できればいい
		if header_field == 'Date':
			continue
		
		# 'Content-Length'はgetしたファイルのサイズと比較する
		if header_field == 'Content-Length':
			if res_header['Content-Length'] != str(data_size):
				return False

		# その他のヘッダーは値が一致する必要ある
		elif res_header[header_field] != expect_header[header_field]:
			print(res_header[header_field])
			return False

	return True


def response_test(path, expected_status, expected_header, file_path, err=0):
	conn = http.client.HTTPConnection("localhost:8000")
	conn.putrequest("POST", path)
	
	# ヘッダーに Transfer-Encoding: chunked を追加
	conn.putheader('Transfer-Encoding', 'chunked')
	conn.endheaders()

	data = create_chunked_data(file_path, err)
	
	# データをチャンクとして送信
	conn.send(data)
	
	res = conn.getresponse()
	assert res.status == expected_status, \
		"Status_code Error" + error_text(expected_status, res.status)
	
	assert header_checker(expected_headers, res.headers, len(raw_data)),\
		"Header Error" + error_text(expected_headers, res.headers)

	assert res.text == data,\
		"Body Error" + error_text(data, res.text)

	print(url + " test done")

def CHUNKED_test():
	# 通常のPOST
	#response_test("/POST/post.txt", 201, SIMPLE_HEADERS, "index.html")
	# ディレクトリを指定してPOST
	#response_test("/POST/", 201, SIMPLE_HEADERS, "index.html")
	# 正しくないデータフォーマット
	response_test("/POST/index.html", 400, SIMPLE_HEADERS, "index.html", err=1)


if __name__ == "__main__":
	CHUNKED_test()
