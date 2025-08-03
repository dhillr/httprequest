// ------------------------------------
// gcc -shared -fPIC src/main.c -o bin/main.so
// ------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

int connect_socket(char* host, in_port_t port) {
    struct hostent* entry = gethostbyname(host);
    struct sockaddr_in addr;
    int on = 1;

    if (!entry) {
        fprintf(stderr, "problem with entry\n");
        return -1;
    }

    memcpy(&addr.sin_addr, entry->h_addr_list[0], entry->h_length);

    addr.sin_port = htons(port); // host byte -> network byte
    addr.sin_family = AF_INET;

    int res = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(res, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));

    if (res == -1) {
        fprintf(stderr, "problem with socket\n");
        return -1;
    }
    if (connect(res, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "problem with connecting socket\n");
        return -1;
    }

    return res;
}

char* send_get_request(char* host, char* page) {
    int fd = connect_socket(host, 80);
    if (fd < 0) return NULL;
    char* buf = malloc(1024);
    char* res = malloc(1024);
    size_t len = 0;
    size_t offset = 0;

    char* request = malloc(strlen("GET ") + strlen(" HTTP/1.0\r\nHost: \r\n\r\n") + strlen(page) + strlen(host));
    strcpy(request, "GET ");
    strcpy(request + strlen("GET "), page);
    strcpy(request + strlen("GET ") + strlen(page), " HTTP/1.0\r\nHost: ");
    strcpy(request + strlen("GET ") + strlen(page) + strlen(" HTTP/1.0\r\nHost: "), host);
    strcpy(request + strlen("GET ") + strlen(page) + strlen(" HTTP/1.0\r\nHost: ") + strlen(host), "\r\n\r\n");

    write(fd, request, strlen(request)); // write a request to the socket

    ssize_t n;
    while((n = read(fd, buf, 1023)) > 0) {
        offset = len;
        len += n;
        res = realloc(res, len);
        memcpy(res + offset, buf, n);
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);

    return res;
}

int main() {
    char* response = send_get_request("example.com", "/");
    printf("%s\n", response);
    return 0;
}