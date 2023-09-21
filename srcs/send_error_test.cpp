#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

const char *ERROR_HTTP_VERSION = "GET / HTTP/1.0\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: Test-Client\r\n"
			"Accept: text/html\r\n\r\n";

const char *NO_NEWLINE = "GET / HTTP/1.1\r\n"
			"Host: localhost:8000\r"
			"User-Agent: Test-Client\r\n"
			"Accept: text/html\r\n\r\n";

const char *NO_END_OF_HEADER = "GET / HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: Test-Client\r\n";

const char *BAD_METHOD_HEADER = "GETTTTT / HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"User-Agent: Test-Client\r\n\r\n";

const char *BAD_FIELD_PAIR_HEADER = "GET / HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"Content-Length: 100\r\n"
			"Transfer-encoding: chunked\r\n"
			"User-Agent: Test-Client\r\n\r\n";

const char *BAD_ENCODE_HEADER = "GET / HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"Transfer-encoding: gzip\r\n"
			"User-Agent: Test-Client\r\n\r\n";

const char *OBS_HEADER = "GET / HTTP/1.1\r\n"
			"Host: localhost:8000\r\n"
			"Transfer-encoding: gzip\r\n"
			"	chunked\r\n"
			"chunkedUser-Agent: Test-Client\r\n\r\n";

int send_http_request(const char *host, int port, const char *request) {
    struct sockaddr_in server_addr;
    int sockfd, bytes_sent, bytes_received;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation error");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection error");
        close(sockfd);
        return -1;
    }

    bytes_sent = send(sockfd, request, strlen(request), 0);
    if (bytes_sent == -1) {
        perror("Send error");
        close(sockfd);
        return -1;
    }

    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(sockfd);
    return 0;
}

int main() {
    const char *host = "127.0.0.1";
    int port = 8000;

	// 505 HTTP Version Not Supported
    send_http_request(host, port, ERROR_HTTP_VERSION);
	printf("\n\n");
	// 400 Bad Request
    send_http_request(host, port, NO_NEWLINE);
	printf("\n\n");
	// 408 Request Time-out
    send_http_request(host, port, NO_END_OF_HEADER);
	printf("\n\n");
	// 501 NOT IMPLEMENT
    send_http_request(host, port, BAD_METHOD_HEADER);
	printf("\n\n");
	// 400 BAD REQUEST
    send_http_request(host, port, BAD_FIELD_PAIR_HEADER);
	printf("\n\n");
	// 501 NOT IMPLEMENT
    send_http_request(host, port, BAD_ENCODE_HEADER);
	printf("\n\n");
	// 400 BAD Request
    send_http_request(host, port, OBS_HEADER);

    return 0;
}
