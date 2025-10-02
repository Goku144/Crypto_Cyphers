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
#include <windows.h>
#include <bcrypt.h>

#if !defined(__CYPHER_KEYS__)
#define __CYPHER_KEYS__

/****************************** Type Definition *****************************/

typedef enum CY_STATE_FLAG {CY_NORMAL, CY_ERROR} CY_STATE_FLAG;
typedef enum CY_PRIMALITY_FLAG {CY_INCONCLUSIVE, CY_COMPOSITE, CY_PRIME} CY_PRIMALITY_FLAG;
typedef enum CY_OWNERSHIP_FLAG {CY_OWNED, CY_NOT_OWNED} CY_OWNERSHIP_FLAG;
typedef enum CY_CRYPT {CY_ENCRYPTION, CY_DECRYPTION} CY_CRYPT;
typedef struct CY_Residu64 {uint64_t value; uint64_t mod;} CY_Residu64;
typedef struct CY_String {uint8_t *str; size_t size; CY_OWNERSHIP_FLAG owner;} CY_String, CY_KEY;
typedef CY_STATE_FLAG (*CY_FUNC)(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer);

/***************** 
 * START HELPERS *
 *****************/

/****************************** Math Functions ******************************/

/*
* Calculate the Greatest Common Divisor for 
* a and b, and they must be positive 
*/
uint64_t gcd(uint64_t a, uint64_t b);

/*
* Calculate the Multiplicative Inverse of a
* modulo n (-a) using the Extended Euclidean
* Algorithm
*/
CY_STATE_FLAG EEA(uint64_t a, uint64_t n, uint64_t *out);

/*
* The Miller–Rabin Algorithm test for
* primality of large numbers returns
* inconclusive if its possible prime,
* composite if its not and MRA_err if
* there is a condition break
*/
CY_STATE_FLAG MRA(uint64_t n, CY_PRIMALITY_FLAG *out);

/*
* The Extended Miller–Rabin Algorithm 
* uses repeated MRA to have better
* result64 with (1/4)^prob error margin
*/
CY_STATE_FLAG EMRA(uint64_t n, uint64_t prob, CY_PRIMALITY_FLAG *outy);

/*
* Calculate the number from its residu64
* using the Chinese Remainder Theorem
*/
CY_STATE_FLAG CRT(const CY_Residu64 a[], uint64_t size, uint64_t *out);

/***************
 * END HELPERS *
 ***************/

/****************************** Cypher Functions ****************************/

/*
* Caesar Cypher uses affine linear key mod 26
* in the form y = keya^(-1)*(x - keyb) mod 26
*/
// CY_STATE_FLAG caesarCypher(const char *inpath, uint8_t keya, uint8_t keyb, CY_CRYPT crypt, const char *outpath);

CY_STATE_FLAG cypher(const char *inpath, CY_KEY *key, const CY_FUNC cypherfunc, const CY_CRYPT crypt, const char *outpath);

CY_STATE_FLAG normal(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer);

CY_STATE_FLAG caesar(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer);

CY_STATE_FLAG monoalphabetic(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer);

#endif // __CYPHER_KEYS__