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

Result gcd(uint64_t a, uint64_t b)
{
    if(!a) return (Result){b,NORMAL};
    if(!b) return (Result){a,NORMAL};

    while (b)
    {
        uint64_t r = a % b;
        a = b; // the next a
        b = r; // the next b
    }
    return (Result) {a,NORMAL};
}

Result EEA(uint64_t a, uint64_t n)
{
    if(gcd(a,n).res != 1) {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return (Result) {0,ERROR};}
    // __t represent t-2 and _t represent t-1
    __int128_t __x = 1, _x = 0, mod = n;
    while (n)
    {
        __int128_t r = (__int128_t) a % n;
        __int128_t x = __x - (a / n)*_x;
        __x = _x; _x = x;
        a = n; 
        n = r;
    }
    return (Result) {(uint64_t) (mod + (__x % mod)) % mod,NORMAL};
}

FLAG MRA(const uint64_t n)
{
    if(n < 3) {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return ERROR;}
    if(!(n % 2)) return EVEN;
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

FLAG EMRA(const uint64_t n, const uint64_t prob) // probability that the Extensive MRA test will return INCONCLUSIVE is (1/4)^(prob)
{
    if (MRA(n) == ERROR) return ERROR;
    if (MRA(n) == EVEN) return EVEN;
    for (uint64_t i = 0; i < prob; i++)
        if(MRA(n) == COMPOSITE)
            return COMPOSITE;
    return INCONCLUSIVE;  
}

Result CRT(const Residu a[], const uint64_t size)
{
    Result result = {0,NORMAL};
    if(!size) {fprintf(stderr, "Can't have null size"); result.flag = ERROR; return result;}
    uint64_t M = 1;
    uint64_t A = 0;

    for (uint64_t i = 0; i < size; i++)
    {
        if(!a[i].mod)
        {fprintf(stderr, "Can't have null modulos"); result.flag = ERROR; return result;}

        Result gcd_v = gcd(a[i].mod,M);
        if(gcd_v.res != 1)
        {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") = %"PRIu64" CRT condition not meet\n", a[i].mod, (uint64_t) M, gcd_v.res); result.flag = ERROR; return result;}

        // check if M has overflowed
        if(M > UINT64_MAX / a[i].mod) result.flag = OVERFLOW;
        M *= a[i].mod;    
    }

    M = M ? M : UINT64_MAX;

    for (uint64_t i = 0; i < size; i++)
    {
        uint64_t Mi = M / a[i].mod;
        uint64_t Mi_inv = EEA((uint64_t) Mi, a[i].mod).res;
        
        __uint128_t t = (Mi * Mi_inv) % M;
        t = (t * (a[i].value % a[i].mod)) % M;

        A = (uint64_t) ((t + A) % M);
    }
    result.res = A % M;
    return result;
}

/**************************** Extraction Functions **************************/



/***************
 * END HELPERS *
 ***************/