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

    /* 1–19: generic */
    CY_ERR,
    CY_ERR_ARG,
    CY_ERR_STATE,
    CY_ERR_UNSUPPORTED,
    CY_ERR_NOT_IMPL,
    CY_ERR_INTERNAL,

    /* 20–39: memory/buffer */
    CY_ERR_OOM,
    CY_ERR_SPACE,     // (add now: caller out buffer too small)
    CY_ERR_SIZE,      // invalid size/len
    CY_ERR_OVERLAP,   // (add now: in/out overlap not allowed)

    /* 40–59: I/O */
    CY_ERR_IO,
    CY_ERR_EOF,
    CY_ERR_OPEN,
    CY_ERR_CLOSE,

    /* 60–79: parsing/validation (handy even if unused today) */
    CY_ERR_FORMAT,    // parse error (hex/base64/etc.)
    CY_ERR_VALUE,     // invalid content (e.g., not a permutation)
    CY_ERR_RANGE,     // out of allowed range

    /* 80–109: crypto-specific */
    CY_ERR_RNG,
    CY_ERR_KEY,
    CY_ERR_KEY_SIZE,
    CY_ERR_KEY_VALUE,

} CY_STATE_FLAG;


typedef enum CY_PRIMALITY_FLAG 
{

    CY_INCONCLUSIVE, 
    CY_COMPOSITE, 
    CY_PRIME

} CY_PRIMALITY_FLAG;

typedef enum CY_OWNERSHIP_FLAG 
{

    CY_OWNED,
    CY_NOT_OWNED

} CY_OWNERSHIP_FLAG;

typedef struct CY_Residu64 
{

    uint64_t value;
    uint64_t mod;

} CY_Residu64;

typedef struct CY_String 
{
    
    uint8_t *str; 
    size_t size; 
    CY_OWNERSHIP_FLAG owner;

} CY_String, CY_KEY;

typedef CY_STATE_FLAG (*CY_FUNC)
(

    CY_String file, 
    CY_KEY *key, 
    uint8_t **buffer
    
);

/************************* linear Key Functions ***************************/

CY_STATE_FLAG cy_key_linear_direct_generate(const uint8_t start, const uint8_t end, CY_KEY *key);

CY_STATE_FLAG CY_key_linear_rand_generated(const uint8_t start, const uint8_t end, CY_KEY *key);

CY_STATE_FLAG cy_key_linear_inverse_generated(const uint8_t start, const uint8_t end, const CY_KEY mapkey, const CY_KEY key, CY_KEY *invkey);

CY_STATE_FLAG cy_key_import(const char *inpath, CY_KEY *key);

CY_STATE_FLAG cy_key_export(const CY_KEY key, const char *outpath);


/**************************** Cypher Functions ****************************/

CY_STATE_FLAG cypher(const char *inpath, CY_KEY *key, const CY_FUNC cypherfunc, const char *outpath);

CY_STATE_FLAG cy_encryption_caesar(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_decryption_caesar(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_encryption_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_decryption_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_crack_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_encryption_eascii(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_decryption_eascii(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_encryption_playfair(CY_String file, CY_KEY *key, uint8_t **buffer);

CY_STATE_FLAG cy_decryption_playfair(CY_String file, CY_KEY *key, uint8_t **buffer);

#endif // __CYPHER_KEYS__