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
    if(!a) return b;
    if(!b) return a;

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

MRA_VAL MRA(uint64_t n)
{
    if(n < 3) {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return MRA_ERR;}
    if(!(n % 2)) return MRA_EVEN;
    if(n == 3) return INCONCLUSIVE;

    __uint128_t a = (__uint128_t) (rand() % (n - 1)); // it make a become born between 0 <= a < n - 1
    if(a < 2) a += 2; // 1 < a < n - 1
    __uint128_t a_q = 1, power = 0, odd = n - 1;

    while (!(odd & 1))
    {
        odd >>= 1; // devid by 2
        power++;
    }

    while (odd) { // fast multiplication using binary base shifting
        if (odd & 1) a_q = (a_q * a) % n;
        a = (a * a) % n;
        odd >>= 1;
    }

    if(a_q == 1) return INCONCLUSIVE;
    for (__uint128_t i = 0; i < power; i++)
    {
        if (a_q == (__uint128_t) (n - 1))
            return INCONCLUSIVE;
        a_q = (a_q * a_q) % n;
    }
    return COMPOSITE;
}

MRA_VAL EMRA(uint64_t n, uint64_t prob) // probability that the Extensive MRA test will return INCONCLUSIVE is (1/4)^(prob)
{
    if (MRA(n) == MRA_ERR) return MRA_ERR;
    if (MRA(n) == MRA_EVEN) return MRA_EVEN;
    for (uint64_t i = 0; i < prob; i++)
        if(MRA(n) == COMPOSITE)
            return COMPOSITE;
    return INCONCLUSIVE;  
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