/*
Christopher Coballes
Hi-Techno Barrio
Bicol,Philippines

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>

#define ESC "\033"

void print_box(const char *text, int width, int height, int fg_color, int bg_color) {
    // Print top border
    printf(ESC "[%d;%dm+", fg_color, bg_color);
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    printf("+\033[0m\n");

    // Print sides with text
    for (int i = 0; i < height - 2; i++) {
        printf(ESC "[%d;%dm| ", fg_color, bg_color);
        for (int j = 0; j < width - 4; j++) {
            putchar(text[i * (width - 4) + j]);
        }
        printf(" |\033[0m\n");
    }

    // Print bottom border
    printf(ESC "[%d;%dm+", fg_color, bg_color);
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    printf("+\033[0m\n");
}

void ftp_client(char *host, char *user, char *pass, char *remote_file, char *local_file) {
    // Resolve hostname
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, "ftp", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return;
    }

    // Create control socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    // Connect to server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        return;
    }

    // Read welcome message
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Send user command
    snprintf(buffer, sizeof(buffer), "USER %s\r\n", user);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("write");
        return;
    }

    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Send password command
    snprintf(buffer, sizeof(buffer), "PASS %s\r\n", pass);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("write");
        return;
    }

    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\
