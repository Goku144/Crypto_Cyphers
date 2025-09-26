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
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#if !defined(__CYPHER_KEYS__)
#define __CYPHER_KEYS__

/****************************** Type Definition *****************************/

typedef enum FLAG {NORMAL, OVERFLOW, ERROR, ODD, EVEN, INCONCLUSIVE, COMPOSITE} FLAG;
typedef struct Result {uint64_t res; FLAG flag;} Result;
typedef struct String {char *str; size_t size;} StringBuffer;
typedef struct Residu {uint64_t value; uint64_t mod;} Residu;

/***************** 
 * START HELPERS *
 *****************/

/****************************** Math Functions ******************************/

/*
* Calculate the Greatest Common Divisor for 
* a and b, and they must be positive 
*/
Result gcd(uint64_t a, uint64_t b);

/*
* Calculate the Multiplicative Inverse of a
* modulo n (-a) using the Extended Euclidean
* Algorithm
*/
Result EEA(uint64_t a, uint64_t n);

/*
* The Miller–Rabin Algorithm test for
* primality of large numbers returns
* inconclusive if its possible prime,
* composite if its not and MRA_err if
* there is a condition break
*/
FLAG MRA(uint64_t n);

/*
* The Extended Miller–Rabin Algorithm 
* uses repeated MRA to have better
* result with (1/4)^prob error margin
*/
FLAG EMRA(uint64_t n, uint64_t prob);

/*
* Calculate the number from its residu
* using the Chinese Remainder Theorem
*/
Result CRT(const Residu a[], uint64_t size);

/**************************** Extraction Functions **************************/

/***************
 * END HELPERS *
 ***************/

#endif // __CYPHER_KEYS__