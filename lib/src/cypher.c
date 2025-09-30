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

/****************************** Random Functions ******************************/

// Generate: FLAG <name>(<T> n, <T>* r)
#define DECL_RANDOM_UNIFORM_UNSIGNED(T, name)                                      \
static FLAG name(T n, T *out) {                                                    \
    if (!out || n == 0) return ERROR_CYPHER;                                       \
    const T max = (T)~(T)0; /* 2^w - 1 */                                          \
    const T lim = (T)(max - (max % n)); /* multiple of n */                        \
    for (;;) {                                                                     \
        NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, (ULONG)sizeof(T),         \
                                      BCRYPT_USE_SYSTEM_PREFERRED_RNG);            \
        if (st != 0) return ERROR_CYPHER;                                          \
        if (*out < lim) { *out = (T)(*out % n); return NORMAL_CYPHER; }            \
    }                                                                              \
}

// declare (NO trailing semicolons)
DECL_RANDOM_UNIFORM_UNSIGNED(uint8_t, random_u8)
DECL_RANDOM_UNIFORM_UNSIGNED(__uint128_t, random_u128)


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

FLAG EEA(uint64_t a, uint64_t n, uint64_t *out)
{
    if(gcd(a,n) != 1) {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return ERROR_CYPHER;}
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
    *out = (mod + (__x % mod)) % mod;
    return NORMAL_CYPHER;
}

FLAG MRA(const uint64_t n, NUMBER_FLAG *out)
{
    *out = UNDEFINED;
    if(n < 3) {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return ERROR_CYPHER;}
    if(!(n % 2)) {*out = EVEN; return NORMAL_CYPHER;}
    if(n == 3)  {*out = INCONCLUSIVE; return NORMAL_CYPHER;}

    __uint128_t a = n - 2; 
    if(random_u128((uint8_t)(n - 1), &a) == ERROR_CYPHER)
        return ERROR_CYPHER; // it make a become born between 0 <= a < n - 1
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
    *out = INCONCLUSIVE;
    if(a_q == 1) return NORMAL_CYPHER;
    for (__uint128_t i = 0; i < power; i++)
    {
        if (a_q == (__uint128_t) (n - 1))
            return NORMAL_CYPHER;
        a_q = (a_q * a_q) % n;
    }
    *out = COMPOSITE;
    return NORMAL_CYPHER;
}

FLAG EMRA(const uint64_t n, const uint64_t prob, NUMBER_FLAG *out) // probability that the Extensive MRA test will return INCONCLUSIVE is (1/4)^(prob)
{
    if (MRA(n,out) == ERROR_CYPHER) return ERROR_CYPHER;
    if (*out == EVEN) return NORMAL_CYPHER;
    for (uint64_t i = 0; i < prob; i++)
        MRA(n,out);
    return NORMAL_CYPHER;  
}

FLAG CRT(const Residu64 a[], const uint64_t size, uint64_t *out)
{
    *out = 0;
    if(!size) {fprintf(stderr, "Can't have null size"); return ERROR_CYPHER;}
    uint64_t M = 1;
    uint64_t A = 0;

    for (uint64_t i = 0; i < size; i++)
    {
        if(!a[i].mod)
        {fprintf(stderr, "Can't have null modulos"); return ERROR_CYPHER;}

        uint64_t gcd_v = gcd(a[i].mod,M);
        if(gcd_v != 1)
        {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") = %"PRIu64" CRT condition not meet\n", a[i].mod, (uint64_t) M, gcd_v); return ERROR_CYPHER;}

        // check if M has overflowed
        if(M > UINT64_MAX / a[i].mod) return OVERFLOW_CYPHER;
        M *= a[i].mod;    
    }

    M = M ? M : UINT64_MAX;

    for (uint64_t i = 0; i < size; i++)
    {
        uint64_t Mi = M / a[i].mod;
        uint64_t Mi_inv = 0;
        if(EEA((uint64_t) Mi, a[i].mod, &Mi_inv) == ERROR_CYPHER)
            return ERROR_CYPHER;

        __uint128_t t = (Mi * Mi_inv) % M;
        t = (t * (a[i].value % a[i].mod)) % M;

        A = (uint64_t) ((t + A) % M);
    }
    *out = A % M;
    return NORMAL_CYPHER;
}

/****************************** File Functions ****************************/

FLAG extTxtFile(const char *path, String *outbuffer)
{
    FLAG state = NORMAL_CYPHER;
    // initialize variables
    *outbuffer = (String) {NULL, 0};
    // opening the file
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {perror("extTxtFile(fopen)"); state = ERROR_CYPHER; goto _return;}

    // extracting the file size
    if (fseek(fp, 0, SEEK_END))
    {perror("extTxtFile(fseek(END))"); state = ERROR_CYPHER; goto _close;}

    long fs = ftell(fp);
    if (fs == -1L)
    {perror("extTxtFile(ftell)"); state = ERROR_CYPHER; goto _close;}
    
    size_t size = (size_t)fs;

    if (fseek(fp, 0, SEEK_SET))
    {perror("extTxtFile(fseek(START))"); state = ERROR_CYPHER; goto _close;}

    // mallocating buffer space
    char *memp = malloc(size + 1);
    if (!memp) 
    {perror("extTxtFile(malloc)"); state = ERROR_CYPHER; goto _close;}

    // read the file
    if (fread(memp, 1, size, fp) != size)
    {perror("extTxtFile(fread)"); state = ERROR_CYPHER; goto _free;}

    outbuffer->str = memp;
    outbuffer->size = size;
    outbuffer->str[size] = '\0';
    goto _close;

    _free:
        free(memp);
    _close:
        if (fclose(fp) != 0) 
        {perror("caesarCypher(fclose)"); state = ERROR_CYPHER;}
    _return:
        return state;
}

static FLAG cypherInit(const char *inpath, String *file, char **buffer)
{
    if(extTxtFile(inpath, file) == ERROR_CYPHER)
        return ERROR_CYPHER;

    *buffer = malloc(file->size + 1);
    if (!*buffer) 
    {perror("malloc"); free(file->str); return ERROR_CYPHER;}

    return NORMAL_CYPHER;
}

static FLAG cypherEnd(const char *outpath, String *file, char *buffer)
{
    FLAG state = NORMAL_CYPHER;

    FILE *fp = fopen(outpath, "wb");
    if (!fp)
    {perror("fopen"); state = ERROR_CYPHER; goto _free;}

    // read the file
    if (fwrite(buffer, 1, file->size, fp) != file->size)
    {perror("write"); state = ERROR_CYPHER;}

    if (fclose(fp) != 0) 
    {perror("fclose"); state = ERROR_CYPHER;}

    _free:
        free(file->str);
        free(buffer);
    return state;
}

/***************
 * END HELPERS *
 ***************/

/****************************** Cypher Functions ****************************/

/******************
 * START TEMPLATE *
 ******************/

/*

FLAG nameCypher(const char *inpath, char *key, CRYPT crypt, const char *outpath)
{
    FLAG state = NORMAL_CYPHER; String file; char *buffer = NULL;

    state = cypherInit(inpath, &file, &buffer);
    if(state == ERROR_CYPHER)
    {fprintf(stderr, "nameCypher(-> ERROR_CYPHER <-)"); goto _freekey;}

    switch (crypt)
    {
        case ENCRYPTION:
            break;
        
        case DECRYPTION:
            break;
        
        default:
            fprintf(stderr, "No such parametre value nameCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
            state = ERROR_CYPHER;
            goto _free;
    }

    state = cypherEnd(outpath, &file, buffer);
    if(state == ERROR_CYPHER)
        fprintf(stderr, "nameCypher(-> ERROR_CYPHER <-)");
    goto _return;

    _free:
        free(file.str);
        free(buffer);
    _freekey:
        free(key);
    _return:
        return state;
}

*/

/****************
 * END TEMPLATE *
 ****************/

FLAG caesarCypher(const char *inpath, uint8_t keya, uint8_t keyb, CRYPT crypt, const char *outpath)
{
    String file; char *buffer = NULL;
    FLAG state = cypherInit(inpath, &file, &buffer);
    if(state == ERROR_CYPHER)
    {fprintf(stderr, "caesarCypher(-> ERROR_CYPHER <-)"); goto _return;}

    uint64_t keyainv;
    if(EEA(keya, 26, &keyainv) == ERROR_CYPHER)
    {state = ERROR_CYPHER; goto _free;}
    keyb %= (uint8_t) 26;

    switch (crypt)
    {
        case ENCRYPTION:
            for (size_t i = 0; i < file.size; i++)
            {
                char c = file.str[i];
                if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    char cas = 'a';
                    if((c >= 'A' && c <= 'Z'))
                        cas = 'A';
                    c = ((keya * (c - cas) + keyb) % 26 + cas);
                }
                buffer[i] = c;
            } 
            break;
        
        case DECRYPTION:
            for (size_t i = 0; i < file.size; i++)
            {
                char c = file.str[i];
                if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    char cas = 'a';
                    if((c >= 'A' && c <= 'Z'))
                        cas = 'A';
                    c = ((keyainv * (c - cas - keyb + 26)) % 26 + cas);
                }
                buffer[i] = c;
            } 
            break;
        
        default:
            fprintf(stderr, "No such parametre value caesarCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
            state = ERROR_CYPHER;
            goto _free;
    }

    state = cypherEnd(outpath, &file, buffer);
    if(state == ERROR_CYPHER)
        fprintf(stderr, "caesarCypher(-> ERROR_CYPHER <-)");
    goto _return;

    _free:
        free(file.str);
        free(buffer);

    _return:
        return state;
}


FLAG monoalphabeticCypher(const char *inpath, char *key, CRYPT crypt, const char *outpath)
{
    FLAG state = NORMAL_CYPHER; String file; char *buffer = NULL;

    state = cypherInit(inpath, &file, &buffer);
    if(state == ERROR_CYPHER)
    {fprintf(stderr, "monoalphabeticCypher(-> ERROR_CYPHER <-)"); goto _freekey;}

    switch (crypt)
    {
        case ENCRYPTION:
            if (!key)
            {
                key = malloc(26);
                if (!key)
                {perror("monoalphabeticCypher(-> malloc -> key <-)"); state = ERROR_CYPHER; goto _return;}
                for (uint8_t i = 0; i < 26; i++) key[i] = 'a' + i;
                for (uint8_t i = 0; i < 26; i++)
                {
                    uint8_t r; 
                    if(random_u8((uint8_t)(i + 1), &r) == ERROR_CYPHER)
                    {state = ERROR_CYPHER; goto _freekey;}
                    char tmp = key[i]; key[i] = key[r]; key[r] = tmp;
                }
            }
        break;

        case DECRYPTION:
            if (!key)
            {
                key = malloc(26);
                if (!key)
                {perror("monoalphabeticCypher(-> malloc -> key <-)"); state = ERROR_CYPHER; goto _return;}
                size_t numtable[26] = {0};
                for (size_t i = 0; i < file.size; i++)
                {
                    char c = file.str[i];
                    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                    {
                        char cas = 'a';
                        if((c >= 'A' && c <= 'Z'))
                            cas = 'A';
                        numtable[c - cas]++;
                    }
                }
                char chartable[26];
                for (size_t i = 0; i < 26; i++) chartable[i] = 'a' + i;
                for (size_t i = 0; i < 26; i++)
                {
                    size_t k = i;
                    for (size_t j = i + 1; j < 26; j++)
                    {
                        if(numtable[k] < numtable[j])
                            k = j;
                    }
                    size_t tmpnum = numtable[i]; numtable[i] = numtable[k]; numtable[k] = tmpnum;
                    char tmpchar = chartable[i]; chartable[i] = chartable[k]; chartable[k] = tmpchar;
                }
                char freqchar[] = "etaoinshrdlcumwfgypbvkjxqz";
                for (size_t i = 0; i < 26; i++)
                    key[chartable[i] - 'a'] = freqchar[i];                
            }
        break;
        
        default:
            fprintf(stderr, "No such parametre value monoalphabeticCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
            state = ERROR_CYPHER;
            goto _free;
    }

    for (size_t i = 0; i < file.size; i++)
    {
       char c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            char cas = 'a';
            if((c >= 'A' && c <= 'Z'))
                cas = 'A';
            c = key[c - cas] - 'a' + cas;
        }
        buffer[i] = c; 
    }

    state = cypherEnd(outpath, &file, buffer);
    if(state == ERROR_CYPHER)
        fprintf(stderr, "monoalphabeticCypher(-> ERROR_CYPHER <-)");
    goto _return;

    _free:
        free(file.str);
        free(buffer);
    _freekey:
        free(key);
    _return:
        return state;
}