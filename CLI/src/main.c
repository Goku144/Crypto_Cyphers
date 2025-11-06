/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <cypher.h>
#include <unistd.h>
#include <fcntl.h> 

// static void print_u128_hex(__uint128_t x) {
//     for (int i=0; i <16; i++) 
//         printf("%c\t", (uint8_t) ((x >> (i * 8)) & 0xFF));
// }

static void cy_add_padding(const size_t len, uint8_t *buffer, uint8_t *pad)
{
    *pad = 16 - len % 16; 
    for (size_t i = 0; i < *pad; i++) buffer[len + i] = (uint8_t) i;
}

static void cy_buff_imp_msg_128(const uint8_t buffer[], __uint128_t *msg)
{
    *msg = 0;
    for (size_t i = 0; i < 16; i++) *msg |= (__uint128_t)(((__uint128_t)buffer[i]) << (i * 8));
}

static void cy_buff_exp_msg_128(const __uint128_t msg, uint8_t buffer[])
{
    for (size_t i = 0; i < 16; i++) buffer[i] = (uint8_t) ((((__uint128_t)msg) >> (i * 8)) & 0xFF);
}

static void cy_buff_imp_size(const uint8_t buffer[], uint64_t *size)
{
    *size = 0;
    for (size_t i = 0; i < 8; i++) *size |= (uint64_t)(((uint64_t)buffer[i]) << (i * 8));
}

static void cy_buff_exp_size(const uint64_t size, uint8_t buffer[])
{
    for (size_t i = 0; i < 8; i++) buffer[i] = (uint8_t) ((((uint64_t)size) >> (i * 8)) & 0xFF);
}

static void cy_buff_imp_size_pad(const uint8_t buffer[], uint64_t *size, uint8_t *pad)
{
    *pad = buffer[8]; *size = 0;
    for (size_t i = 0; i < 8; i++) *size |= (uint64_t)(((uint64_t)buffer[i]) << (i * 8));
}

static void cy_buff_exp_size_pad(const uint64_t size, const uint8_t pad, uint8_t buffer[])
{
    for (size_t i = 0; i < 8; i++) buffer[i] = (uint8_t) ((((uint64_t)size) >> (i * 8)) & 0xFF);
    buffer[8] = pad;
}

static void cy_buff_rsa_key_imp(const uint8_t buffer[], mpz_t **key)
{

}

static void cy_buff_rsa_key_exp(const mpz_t *key, uint8_t buffer[])
{
    // uint64_t size = 0;
    // for (size_t i = 1; i < 3; i++)
    // {
    //     mpz_export (z, 20, 1, sizeof(buffer[0]), 0, 0, a);
    //     cy_buff_exp_size();
    // }
    
}

static void cy_buff_rsa_encryption(const uint8_t buffin[], const mpz_t *key, uint8_t buffout[])
{
    uint64_t size, offset = 0; cy_buff_imp_size(buffin, &size);
    mpz_t cy_msg; mpz_init(cy_msg); size_t msgsize;
    for (size_t i = 0; i < size; i++)
    {
        offset += 8;
        cy_rsa_encryption(buffin[9 + i], key, cy_msg);
        mpz_export(buffout + 9 + offset, &msgsize, 1, 1, 1, 0, cy_msg);
        cy_buff_exp_size((uint64_t) msgsize, buffout + 9 + offset - 8);
        offset += msgsize;
    }
    cy_buff_exp_size_pad(size, 0, buffout); mpz_clear(cy_msg);
}

static void cy_buff_rsa_decryption(const uint8_t buffin[], const mpz_t *key, uint8_t buffout[])
{
    uint64_t size, offset = 8; cy_buff_imp_size(buffin, &size);
    mpz_t cy_msg; mpz_init(cy_msg); size_t msgsize = 0;
    for (size_t i = 0; i < size; i++)
    {
        offset += msgsize;
        cy_buff_imp_size(buffin + 9 + offset - 8, &msgsize);
        mpz_import(cy_msg, msgsize, 1, 1, 1, 0, buffin + 9 + offset);
        cy_rsa_decryption(cy_msg, key, buffout + 9 + i);
        offset += 8;
    }
    cy_buff_exp_size_pad(size, 0, buffout); mpz_clear(cy_msg);
}

static void cy_buff_aes_encryption(const __uint128_t key, uint8_t buffer[])
{
    __uint128_t msg, cy_msg; uint64_t size; uint8_t pad;
    cy_buff_imp_size_pad(buffer, &size, &pad);
    size = (size + pad) / 16;
    for (size_t i = 0; i < size; i++)
    {
        cy_buff_imp_msg_128(buffer + 9 + i * 16, &msg);
        cy_aes_encryption(msg, key, &cy_msg);
        cy_buff_exp_msg_128(cy_msg, buffer + 9 + i * 16);
    }
}

static void cy_buff_aes_decryption(const __uint128_t key, uint8_t buffer[])
{
    __uint128_t msg, cy_msg; uint64_t size; uint8_t pad;
    cy_buff_imp_size_pad(buffer, &size, &pad);
    size = (size + pad) / 16;
    for (size_t i = 0; i < size; i++)
    {
        cy_buff_imp_msg_128(buffer + 9 + i * 16, &cy_msg);
        cy_aes_decryption(cy_msg, key, &msg);
        cy_buff_exp_msg_128(msg, buffer + 9 + i * 16);
    }
}

static CY_STATE_FLAG cy_read(int fd, uint8_t buffer[], const size_t n)
{
    uint8_t pad = 0; ssize_t readn = read(fd, buffer + 9, n - 9);
    if(readn == -1) return cy_state_manager(CY_ERR_IO, __func__, ": error while reading input");
    cy_add_padding(readn, buffer + 9, &pad); buffer[8] = pad;
    cy_buff_exp_size_pad(readn, pad, buffer);
    return CY_OK;
}

static CY_STATE_FLAG cy_write(int fd, uint8_t buffer[])
{
    uint64_t size = 0; uint8_t pad = 0;
    cy_buff_imp_size_pad(buffer, &size, &pad);
    ssize_t writen = write(fd, buffer + 9, size);
    if(writen == -1) return cy_state_manager(CY_ERR_IO, __func__, ": error while writing output");
    return CY_OK;
}

int main(void)
{
    mpz_t *pubk, *prvk;
    cy_rsa_key_gen(100, &pubk, &prvk);
    size_t buff_1_size = 2048;
    uint8_t buff_1[buff_1_size];
    size_t buff_2_size = buff_1_size * 2048;
    uint8_t buff_2[buff_2_size];

    gmp_printf(" e(before) = %Zd\n", prvk[0]);
    gmp_printf(" n(before) = %Zd\n", prvk[1]);
    // cy_buff_rsa_key_exp(prvk, buff_2);
    // mpz_clears(prvk[0], prvk[1], NULL);
    // free(prvk);
    // cy_buff_rsa_key_imp(buff_2, &prvk);
    // gmp_printf(" e(after) = %Zd\n", prvk[0]);
    // gmp_printf(" n(after) = %Zd\n", prvk[1]);
    cy_read(STDIN_FILENO, buff_1, buff_1_size);
    
    cy_buff_rsa_encryption(buff_1, prvk, buff_2);

    // uint64_t size, offset = 0; cy_buff_imp_size(buff_1, &size);
    // mpz_t cy_msg, msg; mpz_inits(cy_msg, msg, NULL); size_t msgsize;
    // for (size_t i = 0; i < size; i++)
    // {
    //     offset += 8;
    //     cy_rsa_encryption(buff_1[9 + i], pubk, cy_msg);
    //     mpz_export(buff_2 + 9 + offset, &msgsize, 1, 1, 1, 0, cy_msg);
    //     cy_buff_exp_size((uint64_t) msgsize, buff_2 + 9 + offset - 8);
    //     offset += msgsize;
    // }
    // cy_buff_exp_size_pad(size, 0, buff_2);


    size_t buff_3_size = buff_1_size;
    uint8_t buff_3[buff_3_size];
    // msgsize = 0;
    // offset = 8;
    // for (size_t i = 0; i < size; i++)
    // {
    //     offset += msgsize;
    //     cy_buff_imp_size(buff_2 + 9 + offset - 8, &msgsize);
    //     mpz_import(cy_msg, msgsize, 1, 1, 1, 0, buff_2 + 9 + offset);
    //     cy_rsa_decryption(cy_msg, prvk, buff_3 + 9 + i);
    //     offset += 8;
    // }
    // cy_buff_exp_size_pad(size, 0, buff_3);
    cy_buff_rsa_decryption(buff_2, pubk, buff_3);
    cy_write(STDOUT_FILENO, buff_3);
    // mpz_export(buffer2 + 9 + offset, &msgsize, 1, 1, 1, 0, cy_msg);
    // mpz_import(cy_msg, msgsize, 1, 1, 1, 0, buffer2 + 9 + offset);
    return 0;
}

