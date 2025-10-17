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
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#if !defined(__CYPHER_KEYS__)
#define __CYPHER_KEYS__

/***************** 
 * START HELPERS *
 *****************/

/****************************** Math Functions ******************************/

/*
* Calculate the great common devider for a and b
* a and b must be positive
*/
uint64_t gcd(uint64_t a, uint64_t b);

/*
* Calculate the addition inverse of number a for (modn)
* it return's -a for (modn)
*/
uint64_t addModInverse(uint64_t a, uint64_t n);

/*
* Calculate the multiplication inverse of number a for (modn)
* it return's a^(-1) for (modn), gcd(a, n) must be 1
*/
uint64_t mulModInverse(uint64_t a, uint64_t n);

/****************************** Boolean Functions ******************************/

/*
*return 1 if they are the same class (modn), return 0 else
*/
int isSameModClass(uint64_t a, uint64_t b, uint64_t n);

int hasModInverse(uint64_t a, uint64_t b);

/***************
 * END HELPERS *
 ***************/
#endif // __CYPHER_KEYS__