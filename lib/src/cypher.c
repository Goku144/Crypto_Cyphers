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


#include <stdbool.h>
#include "cypher.h"

/***************** 
 * START HELPERS *
 *****************/

/****************************** Math Functions ******************************/

uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b) {uint64_t r = a % b; a = b; b = r;}
    return a;
}

uint64_t addModInverse(uint64_t a, uint64_t n)
{
    return n - a % n;
}

uint64_t mulModInverse(uint64_t a, uint64_t n)
{
    if(!hasModInverse(a, n))
        return 0;
    int64_t __x = 1, _x = 0, __y = 0, _y = 1;
    int64_t __r = (int64_t) a, _r = (int64_t) n;
    while (_r)
    {
        int64_t q = __r / _r;
        int64_t r = __r - q * _r;
        int64_t x = __x - q * _x;
        int64_t y = __y - q * _y;
        __x = _x; _x = x;
        __y = _y; _y = y;
        __r = _r; _r = r;
    }
    return (uint64_t)((__x % (int64_t)n + (int64_t)n) % (int64_t)n);
}

/****************************** Boolean Functions ******************************/

int isSameModClass(uint64_t a, uint64_t b, uint64_t n)
{
    return n ? (a % n == b % n) : (a == b);
}

int hasModInverse(uint64_t a, uint64_t n)
{
    return gcd(a,n) == 1;
}

/***************
 * END HELPERS *
 ***************/