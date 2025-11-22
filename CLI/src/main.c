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
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>


#define CY_HEADER_OFFSET 16
#define CY_BUFFSIZE 2048

struct CY_HEADER
{
    uint64_t cy_data_len;
    uint8_t cy_pad_flag;
    uint8_t cy_pad_size;
    uint8_t cy_enc_flag;
    uint8_t cy_enc_type;
    uint8_t cy_key_flag;
    uint8_t cy_key_type;
    uint8_t cy_hash_flag;
    uint8_t cy_hash_type;
};


void cy_buff_header_exp(const struct CY_HEADER head, uint8_t buff[]);

void cy_buff_header_imp(const uint8_t buff[], struct CY_HEADER *head);

void cy_buff_size_exp(const size_t size, uint8_t buff[]);

void cy_buff_size_imp(const uint8_t buff[], size_t *size);

void serror(const char *fmt);


void cy_buff_size_exp(const size_t size, uint8_t buff[])
{
    for (size_t i = 0; i < sizeof(size_t); i++)
        buff[sizeof(size_t) - (i + 1)] = (uint8_t)((size >> (i * 8)) & 0xFF);  
}

void cy_buff_size_imp(const uint8_t buff[], size_t *size)
{
    *size = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) 
        *size |= ((size_t) (buff[sizeof(size_t) - (i + 1)])) << (i * 8);
}

void cy_buff_header_exp(const struct CY_HEADER head, uint8_t buff[])
{
    cy_buff_size_exp(head.cy_data_len, buff);
    buff[8]  = head.cy_pad_flag;  buff[9]  = head.cy_pad_size;
    buff[10] = head.cy_enc_flag;  buff[11] = head.cy_enc_type;
    buff[12] = head.cy_key_flag;  buff[13] = head.cy_key_type;
    buff[14] = head.cy_hash_flag; buff[15] = head.cy_hash_type;
}

void cy_buff_header_imp(const uint8_t buff[], struct CY_HEADER *head)
{
    memset(head, 0, sizeof (*head));
    cy_buff_size_imp(buff, &head->cy_data_len);
    head->cy_pad_flag  = buff[8];  head->cy_pad_size  = buff[9];
    head->cy_enc_flag  = buff[10]; head->cy_enc_type  = buff[11];
    head->cy_key_flag  = buff[12]; head->cy_key_type  = buff[13];
    head->cy_hash_flag = buff[14]; head->cy_hash_type = buff[15];
}

void cy_buff_send(int __fd, const struct CY_HEADER head, uint8_t *buff)
{
    cy_buff_header_exp(head, buff);
    size_t fullsize = head.cy_data_len + 16;
    size_t total = 0;
    while (total < fullsize)
    {
        ssize_t n = send(__fd, buff + total, fullsize - total, 0);
        if(n == 0) serror("cy_buff_send(-> send <-)");
        if(n < 0) serror("cy_buff_send(-> send <-)");
        total +=n;
    }
}

void cy_buff_recv(int __fd, struct CY_HEADER *head, uint8_t **buff)
{
    size_t fullsize = 16;
    size_t total = 0;
    memset(head, 0, sizeof(*head));
    *buff = malloc(CY_BUFFSIZE * sizeof(**buff));
    if(!*buff) serror("cy_buff_read(-> malloc <-)");
    while (total < fullsize)
    {
        ssize_t n = recv(__fd, *buff + total, fullsize - total, 0);
        if(n == 0) serror("cy_buff_recv(-> recv <-)");
        if(n < 0) serror("cy_buff_recv(-> recv <-)");
        total +=n;
        if(total == 16)
        {
            cy_buff_header_imp(*buff, head);
            fullsize = head->cy_data_len + total;
        }
    }
}

void cy_buff_read(int __fd, struct CY_HEADER *head, uint8_t **buff)
{
    size_t buffsize = CY_BUFFSIZE;
    memset(head, 0, sizeof(*head));
    *buff = malloc(CY_BUFFSIZE * sizeof(**buff));
    if(!*buff) serror("cy_buff_read(-> malloc <-)");
    while (1)
    {
        if(buffsize - head->cy_data_len < CY_BUFFSIZE) 
        {
            buffsize += CY_BUFFSIZE;
            *buff = realloc(*buff, buffsize * sizeof(*buff));
            if(!*buff) serror("cy_buff_read(-> realloc <-)");
        }
        ssize_t n = read(__fd, *buff + head->cy_data_len + 16, CY_BUFFSIZE - 16);
        if(n == 0) return;
        if(n < 0) serror("cy_buff_read(-> read <-)");
        head->cy_data_len += n;
    }
}

void cy_buff_write(int __fd, const struct CY_HEADER head, const uint8_t *buff)
{
    size_t fullsize = head.cy_data_len;
    size_t total = 0;
    while (total < fullsize)
    {
        ssize_t n = write(__fd, buff + total, fullsize - total);
        if(n == 0) serror("cy_buff_write(-> write <-)");
        if(n < 0) serror("cy_buff_write(-> write <-)");
        total +=n;
    }
}

void serror(const char *fmt)
{
    perror(fmt);
    exit(1);
}

void print_u128_hex(__uint128_t x) {
    unsigned long long hi = (unsigned long long)(x >> 64);
    unsigned long long lo = (unsigned long long)x;

    // 16 hex digits per 64 bits, zero-padded
    printf("0x%016llx%016llx\n", hi, lo);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage:\n  %s -sp <port>      (server)\n  %s <host> <port>   (client)\n",
                argv[0], argv[0]);
        return 1;
    }

    struct sockaddr_in client_addr;
    struct addrinfo hints, *servinfo = NULL, *clientinfo = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP

    if (!strcmp(argv[1], "-sp")) {
        // -------- SERVER MODE --------
        hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(NULL, argv[2], &hints, &servinfo) != 0)
            return 1;  // forget detailed error handling

        int sd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if (sd < 0) return 1;

        if (bind(sd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) return 1;
        if (listen(sd, 1) < 0) return 1;

        printf("listening ...\n");

        socklen_t addr_size = sizeof client_addr;
        int new_fd = accept(sd, (struct sockaddr *)&client_addr, &addr_size);

        if (new_fd == -1)
            printf("Connection was unsuccessfull\n");
        else
            printf("Connection was successful\n");
        struct CY_HEADER head; uint8_t *buff;
        cy_buff_recv(new_fd, &head, &buff);
        cy_buff_write(STDOUT_FILENO, head, buff + CY_HEADER_OFFSET);

        // clean up
        close(new_fd);
        close(sd);
        freeaddrinfo(servinfo);

    } else {
        // -------- CLIENT MODE --------
        if (getaddrinfo(argv[1], argv[2], &hints, &clientinfo) != 0)
            return 1;

        int sd = socket(clientinfo->ai_family, clientinfo->ai_socktype, clientinfo->ai_protocol);
        if (sd < 0) return 1;

        if (connect(sd, clientinfo->ai_addr, clientinfo->ai_addrlen) < 0)
            printf("Connection failed\n");
        else
            printf("Connected to server\n");
        struct CY_HEADER head;
        uint8_t *buff;
        cy_buff_read(STDIN_FILENO, &head, &buff);
        cy_buff_send(sd, head, buff);

        close(sd);
        freeaddrinfo(clientinfo);
    }

    return 0;
}
