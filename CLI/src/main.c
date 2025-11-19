/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <cypher.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

void serror(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);           // start reading ...
    vfprintf(stderr, fmt, ap);   // print to stderr using the va_list
    va_end(ap);

    exit(1);
}

void print_u128_hex(__uint128_t x) {
    unsigned long long hi = (unsigned long long)(x >> 64);
    unsigned long long lo = (unsigned long long)x;

    // 16 hex digits per 64 bits, zero-padded
    printf("0x%016llx%016llx\n", hi, lo);
}

#define BUFFSIZE 4096*128
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr,"usage server: cypher -sp <port>\nusage server: cypher <ip> -p <port>\n");
        return 1;
    }
    int sockfd, sockclientfd;
    struct sockaddr_storage clientaddr;
    socklen_t addr_size;
    struct addrinfo hints, *servinfo, *clientinfo;
    char ipvx[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(!strcmp(argv[1], "-sp"))
    {
        hints.ai_flags = AI_PASSIVE;
        getaddrinfo(NULL, argv[2], &hints, &servinfo);

        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

        bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

        inet_ntop(servinfo->ai_family, &((struct sockaddr_in *)servinfo->ai_addr)->sin_addr, ipvx, INET_ADDRSTRLEN);

        printf("listening on %s %s\n", ipvx, argv[2]);

        listen(sockfd, 1);

        sockclientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addr_size);
        
        if(sockclientfd == -1)
            perror("accept(-> error <-)");
        else
            printf("Connection Was Successfull!\n");
        mpz_t *pubk = malloc(2*sizeof(mpz_t)), *prvk = malloc(2*sizeof(mpz_t));
        mpz_inits(pubk[0], pubk[1], prvk[0], prvk[1], NULL);
        cy_rsa_key_gen(1024, &pubk, &prvk);
        uint8_t buff[1024*128];
        cy_buff_rsa_key_exp(pubk, buff);
        send(sockclientfd, buff, 1024*128, 0);
        gmp_printf("generated in server:\ne=%Zd\nn=%Zd\n", pubk[0], pubk[1]);
        uint8_t buff2[1024*128];
        recv(sockclientfd, buff2, 1024*128, 0);
        __uint128_t aesk;
        cy_buff_rsa_decryption(128, prvk, 1024*128, buff2);
        cy_buff_msg_128_imp(buff2, &aesk);
        printf("from client: ");
        print_u128_hex(aesk);
    }
    else
    {
        getaddrinfo(argv[1], argv[2], &hints, &clientinfo);

        sockfd = socket(clientinfo->ai_family, clientinfo->ai_socktype, clientinfo->ai_protocol);

        connect(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen);

        mpz_t *pubk = malloc(2*sizeof(mpz_t));
        uint8_t buff[1024*128];
        recv(sockfd, buff, 1024*128, 0);
        cy_buff_rsa_key_imp(buff, &pubk);
        gmp_printf("from server:\ne=%Zd\nn=%Zd\n", pubk[0], pubk[1]);
        __uint128_t aesk;
        cy_aes_key_gen(&aesk);
        uint8_t buff2[1024*128];
        cy_buff_msg_128_exp(aesk, buff2);
        cy_buff_rsa_encryption(128, pubk, 1024*128, buff2);
        send(sockfd, buff2, 1024*128, 0);
        printf("generated in client: ");
        print_u128_hex(aesk);
    }

    return 0;
}