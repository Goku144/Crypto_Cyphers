/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cypher.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifndef NI_MAXHOST
#  define NI_MAXHOST 1025
#endif
#ifndef NI_MAXSERV
#  define NI_MAXSERV 32
#endif

static void print_u128_hex(__uint128_t x) 
{
    for (int i=0; i <16; i++) 
        printf("%c\t", (uint8_t) ((x >> (i * 8)) & 0xFF));
}

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static int listen_on(const char *port) {
    struct addrinfo hints, *res, *rp;
    int s, fd = -1, yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;     // v4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;    // for bind

    if ((s = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = res; rp; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0) break; // bound!
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) die("bind");

    if (listen(fd, 16) < 0) die("listen");
    return fd;
}

static int connect_to(const char *host, const char *port) {
    struct addrinfo hints, *res, *rp;
    int s, fd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;   // v4 or v6
    hints.ai_socktype = SOCK_STREAM;

    if ((s = getaddrinfo(host, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = res; rp; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break; // connected!
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) die("connect");
    return fd;
}

static void print_peer(int fd, const char *prefix) {
    char host[NI_MAXHOST], serv[NI_MAXSERV];
    struct sockaddr_storage ss;
    socklen_t slen = sizeof ss;
    if (getpeername(fd, (struct sockaddr*)&ss, &slen) == 0 &&
        getnameinfo((struct sockaddr*)&ss, slen, host, sizeof host,
                    serv, sizeof serv, NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
        fprintf(stderr, "%s %s:%s\n", prefix, host, serv);
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3 && strcmp(argv[1], "-s") == 0) {
        // -------- Server mode: ./app -s <port>
        const char *port = argv[2];
        int lfd = listen_on(port);
        fprintf(stderr, "Server listening on port %s …\n", port);

        struct sockaddr_storage cli;
        socklen_t clen = sizeof cli;
        int cfd = accept(lfd, (struct sockaddr*)&cli, &clen);
        if (cfd < 0) die("accept");
        print_peer(cfd, "Client connected from");

        char buf[CY_BUFSZ];
        for (;;) {
            ssize_t n = recv(cfd, buf, sizeof buf - 1, 0);
            if (n == 0) { fprintf(stderr, "Client closed.\n"); break; }
            if (n < 0) die("recv");
            buf[n] = '\0';
            printf("Client: %s", buf);   // may already contain newline

            // Echo back exactly what was received
            size_t to_send = (size_t)n;
            size_t off = 0;
            while (off < to_send) {
                ssize_t w = send(cfd, buf + off, to_send - off, 0);
                if (w < 0) die("send");
                off += (size_t)w;
            }

            // Quit if the client said "Bye"
            if (n >= 3 && strncmp(buf, "Bye", 3) == 0) break;
        }
        close(cfd);
        close(lfd);
        return 0;
    }

    if (argc == 3) {
        // -------- Client mode: ./app <host> <port>
        const char *host = argv[1];
        const char *port = argv[2];
        int fd = connect_to(host, port);
        print_peer(fd, "Connected to");

        char buf[CY_BUFSZ];
        for (;;) {
            // read a line from stdin
            if (!fgets(buf, sizeof buf, stdin)) break;

            // send it (robust write loop)
            size_t len = strlen(buf), off = 0;
            while (off < len) {
                ssize_t w = send(fd, buf + off, len - off, 0);
                if (w < 0) die("send");
                off += (size_t)w;
            }

            // receive reply (simple echo protocol)
            ssize_t n = recv(fd, buf, sizeof buf - 1, 0);
            if (n <= 0) { fprintf(stderr, "Server closed/recv error\n"); break; }
            buf[n] = '\0';
            printf("Server: %s", buf);

            if (strncmp(buf, "Bye", 3) == 0) break;
        }
        shutdown(fd, SHUT_RDWR);
        close(fd);
        return 0;
    }

    fprintf(stderr,
        "Usage:\n"
        "  Server: %s -s <port>\n"
        "  Client: %s <host> <port>\n", argv[0], argv[0]);
    return 2;
}