import http.client

conn = http.client.HTTPConnection("localhost:8000")
conn.putrequest("POST", "/path")

# ヘッダーに Transfer-Encoding: chunked を追加
conn.putheader('Transfer-Encoding', 'chunked')
conn.endheaders()

# ファイルからデータを読み取り
with open('../hoge.txt', 'rb') as f:
    data = f.read()

# データをチャンクとして送信
conn.send(data)
conn.send(b'\r\n0\r\n\r\n')

response = conn.getresponse()
print(response.status, response.reason)

