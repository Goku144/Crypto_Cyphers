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

#include "cypher.h"

/***************** 
 * START HELPERS *
 *****************/

/****************************** Math Functions ******************************/

uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b)
    {
        uint64_t r = a % b;
        a = b; // the next a
        b = r; // the next b
    }
    return a;
}

uint64_t additiveModInverse(uint64_t a, uint64_t n)
{
    return n - a % n;
}

uint64_t EEA(uint64_t a, uint64_t n)
{
    if(!hasMulModInverse(a,n)) {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different then 1\n", a, n); return 0;}
    // __t represent t-2 and _t represent t-1
    int64_t __x = 1, _x = 0, mod = n;
    while (n)
    {
        int64_t r = (int64_t) a % n;
        int64_t x = __x - (a / n)*_x;
        __x = _x; _x = x;
        a = n; 
        n = r;
    }
    return (uint64_t) (mod + (__x % mod)) % mod;
}

/***************************** Boolean Functions ****************************/

uint64_t isCongruent(uint64_t a, uint64_t b, uint64_t n)
{
    return a % n == b % n;
}

uint64_t hasMulModInverse(uint64_t a, uint64_t n)
{
    return gcd(a,n) == 1;
}

/**************************** Extraction Functions **************************/



/***************
 * END HELPERS *
 ***************/