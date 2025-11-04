/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <cypher.h>

static void print_u128_hex(__uint128_t x) {
    for (int i=0; i <16; i++) 
        printf("%c\t", (uint8_t) ((x >> (i) * 8) & 0xFF));
}

static void add_padding(ssize_t *len, char *buffer)
{
    uint32_t n = *len % 16;
    if(n == 0) return;
    n = 16 - n;
    for (size_t i = 0; i < n; i++)
        buffer[*len - 1 + i] = (char) 'a';
    *len += n - 1;
}

int main(void)
{
    __uint128_t key, msg, cy_msg;
    cy_aes_key_imp("aes.key", &key);
    
    printf("here: ");
    char *buffer= NULL; size_t n = 4096;
    ssize_t size = 0;
    size = getline(&buffer, &n, stdin);
    add_padding(&size, buffer);
    for (size_t i = 0; i < (size_t) size; i++)
        printf("%c", buffer[i]);
    printf("\n");
    return 0;
}

