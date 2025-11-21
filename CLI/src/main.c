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

void cy_buff_send(int __fd, const struct CY_HEADER head, void *buff)
{
    uint8_t *p = buff;
    cy_buff_header_exp(head, p);
    size_t fullsize = head.cy_data_len + 16;
    size_t total = 0;
    while (total < fullsize)
    {
        ssize_t n = send(__fd, p + total, fullsize - total, 0);
        if(n == 0) serror("Peer closed before we send the full msg");
        if(n < 0) serror(__func__);
        total +=n;
    }
}

void cy_buff_recv(int __fd, struct CY_HEADER *head, void *buff)
{
    uint8_t *p = buff;
    size_t fullsize = 16;
    size_t total = 0;
    while (total < fullsize)
    {
        ssize_t n = recv(__fd, p + total, fullsize - total, 0);
        if(n == 0) serror("Peer closed before we recv the full msg");
        if(n < 0) serror(__func__);
        total +=n;
        if(total == 16)
        {
            cy_buff_header_imp(buff, head);
            fullsize = head->cy_data_len + total;
        }
    }
}

void cy_buff_read(int __fd, struct CY_HEADER *head, void **buff)
{
    size_t buffsize = CY_BUFFSIZE;
    memset(head, 0, sizeof(*head));
    uint8_t *p = malloc(CY_BUFFSIZE * sizeof(*p));
    *buff = p;
    if(!p) serror(__func__);
    while (1)
    {
        if(buffsize - head->cy_data_len < CY_BUFFSIZE) 
        {
            buffsize += CY_BUFFSIZE;
            p = realloc(*buff, buffsize * sizeof(*p));
            *buff = p;
            if(!p) serror(__func__);
        }
        ssize_t n = read(__fd, p + head->cy_data_len, CY_BUFFSIZE);
        if(n == 0) return;
        if(n < 0) serror(__func__);
        head->cy_data_len += n;
    }
}

void cy_buff_write(int __fd, const struct CY_HEADER head, void *buff)
{
    uint8_t *p = buff;
    size_t fullsize = head.cy_data_len;
    size_t total = 0;
    while (total < fullsize)
    {
        ssize_t n = send(__fd, p + total, fullsize - total, 0);
        if(n == 0) serror("Peer closed before we send the full msg");
        if(n < 0) serror(__func__);
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

int main(void)
{
    struct CY_HEADER hmida;
    uint8_t buff[CY_BUFFSIZE];
    memset(&hmida, 0, sizeof (hmida));
    hmida.cy_data_len = 777;
    cy_buff_header_exp(hmida, buff);
    struct CY_HEADER hmida2;
    cy_buff_header_imp(buff, &hmida2);
    printf("%zu\n", hmida2.cy_data_len);
    return 0;
}