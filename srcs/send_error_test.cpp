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
			"Host: localhost:8000\r"
			"User-Agent: Test-Client\r\n";

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

    send_http_request(host, port, ERROR_HTTP_VERSION);
	printf("\n\n");
    send_http_request(host, port, NO_NEWLINE);
	printf("\n\n");
    send_http_request(host, port, NO_END_OF_HEADER);

    return 0;
}