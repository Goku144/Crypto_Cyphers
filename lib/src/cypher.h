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

/************************* linear Key Functions ***************************/

CY_STATE_FLAG cy_rsa_key_gen(const mp_bitcnt_t bitsize, mpz_t *pubkey, mpz_t *prvkey);

CY_STATE_FLAG cy_rsa_key_imp(const char *path, mpz_t *key[2]);

CY_STATE_FLAG cy_rsa_key_exp(const char *path, const mpz_t key[2]);

CY_STATE_FLAG cy_aes_key_gen(__uint128_t *key);

CY_STATE_FLAG cy_aes_key_imp(const char *path, __uint128_t *key);

CY_STATE_FLAG cy_aes_key_exp(const char *path, __uint128_t key);

void cy_aes_from_128_to_4by4(const __uint128_t num, uint8_t tab[4][4]);

void cy_aes_substitute_bytes(const uint8_t sbox[16][16], uint8_t state[4][4]);

void cy_aes_shift_rows(uint8_t state[4][4]);

void cy_aes_invshift_rows(uint8_t state[4][4]);

void cy_aes_mix_columns(const uint8_t mix_c_matrix[4][4], uint8_t state[4][4]);

void cy_aes_add_round_key(const uint32_t key[4], uint8_t state[4][4]);

void cy_aes_g_function(const uint8_t j, uint32_t *w);

void cy_aes_key_expansion(__uint128_t key, uint32_t w[44]);

void cy_aes_from_4by4_to_128(const uint8_t tab[4][4], __uint128_t *num);

/**************************** Cypher Functions ****************************/

void cy_rsa_encryption(const char c, const mpz_t *key, mpz_ptr msg);

void cy_rsa_decryption(const mpz_srcptr msg, const mpz_t *key, char *c);

void cy_aes_encryption(__uint128_t msg, __uint128_t key, __uint128_t *cy_msg);

void cy_aes_decryption(__uint128_t msg, __uint128_t key, __uint128_t *cy_msg);

#endif // __CYPHER_KEYS__