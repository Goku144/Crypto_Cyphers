/*
 * Copyright (c) 2025 Jebbari Marouane
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <gmp.h>

#if !defined(__CYPHER_KEYS__)
#define __CYPHER_KEYS__

/****************************** Type Definition *****************************/

typedef enum CY_STATE_FLAG 
{

    CY_OK,

    /* generic */
    CY_ERR,
    CY_ERR_ARG,
    CY_ERR_STATE,
    CY_ERR_UNSUPPORTED,
    CY_ERR_NOT_IMPL,
    CY_ERR_INTERNAL,

    /* memory/buffer */
    CY_ERR_OOM,
    CY_ERR_SPACE,     // (add now: caller out buffer too small)
    CY_ERR_SIZE,      // invalid size/len
    CY_ERR_OVERLAP,   // (add now: in/out overlap not allowed)

    /* I/O */
    CY_ERR_IO,
    CY_ERR_EOF,
    CY_ERR_OPEN,
    CY_ERR_CLOSE,

    /* parsing/validation (handy even if unused today) */
    CY_ERR_FORMAT,    // parse error (hex/base64/etc.)
    CY_ERR_VALUE,     // invalid content (e.g., not a permutation)
    CY_ERR_RANGE,     // out of allowed range

    /* crypto-specific */
    CY_ERR_RNG,
    CY_ERR_KEY,
    CY_ERR_KEY_SIZE,
    CY_ERR_KEY_VALUE,

    /* Info */
    CY_INFO_EOF

} CY_STATE_FLAG;

typedef struct CY_String 
{
    mpz_t *key; 
    size_t size;
} CY_String, CY_KEY;

typedef enum CY_CYPHER_TYPE
{
    CY_RSA,
    CY_AES
} CY_CYPHER_TYPE;


/**************************** flow Functions ******************************/

CY_STATE_FLAG cy_state_manager(const CY_STATE_FLAG e, const char *funcname, const char *msg);

/************************* linear Key Functions ***************************/

CY_STATE_FLAG cy_rsa_key_gen(const mp_bitcnt_t bitsize, mpz_t **pubkey, mpz_t **prvkey);

CY_STATE_FLAG cy_rsa_key_imp(const char *path, mpz_t **key);

CY_STATE_FLAG cy_rsa_key_exp(const char *path, const mpz_t *key);

CY_STATE_FLAG cy_aes_key_gen(__uint128_t *key);

CY_STATE_FLAG cy_aes_key_imp(const char *path, __uint128_t *key);

CY_STATE_FLAG cy_aes_key_exp(const char *path, __uint128_t key);

/**************************** Cypher Functions ****************************/

void cy_rsa_encryption(const uint8_t c, const mpz_t *key, mpz_ptr cy_msg);

void cy_rsa_decryption(const mpz_srcptr cy_msg, const mpz_t *key, uint8_t *c);

void cy_aes_encryption(__uint128_t msg, __uint128_t key, __uint128_t *cy_msg);

void cy_aes_decryption(__uint128_t msg, __uint128_t key, __uint128_t *cy_msg);

/************************* Buffer Cypher Functions ************************/

// void cy_buff_padd16(const size_t size, uint8_t *pad, uint8_t buffer[]);

// void cy_buff_size_exp(const size_t size, uint8_t buff[]);

// void cy_buff_size_imp(const uint8_t buff[], size_t *size);

// void cy_buff_msg_128_exp(const __uint128_t msg, uint8_t buff[]);

// void cy_buff_msg_128_imp(const uint8_t buff[], __uint128_t *size);

// void cy_buff_rsa_key_exp(mpz_t *key, uint8_t buff[]);

// void cy_buff_rsa_key_imp(const uint8_t buff[], mpz_t **key);

// void cy_buff_aes_key_exp(const __uint128_t key, uint8_t buff[]);

// void cy_buff_aes_key_imp(const uint8_t buff[], __uint128_t *key);

// void cy_buff_rsa_encryption(const size_t size, const mpz_t *key, const size_t buffn, uint8_t buff[]);

// void cy_buff_rsa_decryption(const size_t size, const mpz_t *key, const size_t buffn, uint8_t buff[]);

// void cy_buff_aes_encryption(const size_t size, const __uint128_t key, uint8_t buff[]);

// void cy_buff_aes_decryption(const size_t size, const __uint128_t key, uint8_t buff[]);

#endif // __CYPHER_KEYS__