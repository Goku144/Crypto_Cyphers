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
#include <stdint.h>
#include <inttypes.h>

#if !defined(__CYPHER_KEYS__)
#define __CYPHER_KEYS__

/****************************** Type Definition *****************************/

typedef struct String {char *str; size_t size;} StringBuffer; 
typedef struct Pair {uint64_t power; uint64_t odd;} Pair;
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
* Calculate the additive Inverse of a
* modulo n (-a)
*/
uint64_t additiveModInverse(uint64_t a, uint64_t n);

/*
* Calculate the Multiplicative Inverse of a
* modulo n (-a) using the Extended Euclidean
* Algorithm
*/
uint64_t EEA(uint64_t a, uint64_t n);


Pair MRA(uint64_t n);

/***************************** Boolean Functions ****************************/

/*
* Returns 1 if a is Congruent b modulo n
* Returns 0 if not
*/
uint64_t isCongruent(uint64_t a, uint64_t b, uint64_t n);

/*
* Returns 1 if a has inverse modulo n
* Returns 0 if not
*/
uint64_t hasMulModInverse(uint64_t a, uint64_t n);

/**************************** Extraction Functions **************************/



/***************
 * END HELPERS *
 ***************/

#endif // __CYPHER_KEYS__