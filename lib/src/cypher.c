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

#define DECL_RANDOM_UNIFORM_UNSIGNED(T, name)                                      \
static CY_STATE_FLAG name(const T n, T *out) {                                       \
    if (!out || n == 0) return CY_ERROR;                                       \
    const T max = (T)~(T)0; /* 2^w - 1 */                                          \
    const T lim = (T)(max - (max % n)); /* multiple of n */                        \
    for (;;) {                                                                     \
        NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, (ULONG)sizeof(T),         \
                                      BCRYPT_USE_SYSTEM_PREFERRED_RNG);            \
        if (st != 0) return CY_ERROR;                                          \
        if (*out < lim) { *out = (T)(*out % n); return CY_NORMAL; }            \
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

CY_STATE_FLAG EEA(uint64_t a, uint64_t n, uint64_t *out)
{
    if(gcd(a,n) != 1) {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return CY_ERROR;}
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
    return CY_NORMAL;
}

CY_STATE_FLAG MRA(const uint64_t n, CY_PRIMALITY_FLAG *out)
{
    *out = CY_INCONCLUSIVE;
    if(n < 3) {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return CY_ERROR;}
    if(!(n % 2) || n == 3) return CY_NORMAL;

    __uint128_t a = n - 2; 
    if(random_u128((uint8_t)(n - 1), &a) == CY_ERROR)
        return CY_ERROR; // it make a become born between 0 <= a < n - 1
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

    if(a_q == 1) return CY_NORMAL;
    for (__uint128_t i = 0; i < power; i++)
    {
        if (a_q == (__uint128_t) (n - 1))
            return CY_NORMAL;
        a_q = (a_q * a_q) % n;
    }
    *out = CY_COMPOSITE;
    return CY_NORMAL;
}

CY_STATE_FLAG EMRA(const uint64_t n, const uint64_t prob, CY_PRIMALITY_FLAG *out) // probability that the Extensive MRA test will return CY_INCONCLUSIVE is (1/4)^(prob)
{
    if (MRA(n,out) == CY_ERROR) return CY_ERROR;
    for (uint64_t i = 0; i < prob; i++)
        MRA(n,out);
    return CY_NORMAL;  
}

CY_STATE_FLAG CRT(const CY_Residu64 a[], const uint64_t size, uint64_t *out)
{
    *out = 0;
    if(!size) {fprintf(stderr, "Can't have null size"); return CY_ERROR;}
    uint64_t M = 1;
    uint64_t A = 0;

    for (uint64_t i = 0; i < size; i++)
    {
        if(!a[i].mod)
        {fprintf(stderr, "Can't have null modulos"); return CY_ERROR;}

        uint64_t gcd_v = gcd(a[i].mod,M);
        if(gcd_v != 1)
        {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") = %"PRIu64" CRT condition not meet\n", a[i].mod, (uint64_t) M, gcd_v); return CY_ERROR;}

        // check if M has overflowed
        if(M > UINT64_MAX / a[i].mod) return CY_ERROR;
        M *= a[i].mod;    
    }

    M = M ? M : UINT64_MAX;

    for (uint64_t i = 0; i < size; i++)
    {
        uint64_t Mi = M / a[i].mod;
        uint64_t Mi_inv = 0;
        if(EEA((uint64_t) Mi, a[i].mod, &Mi_inv) == CY_ERROR)
            return CY_ERROR;

        __uint128_t t = (Mi * Mi_inv) % M;
        t = (t * (a[i].value % a[i].mod)) % M;

        A = (uint64_t) ((t + A) % M);
    }
    *out = A % M;
    return CY_NORMAL;
}

/***************************** File Functions *****************************/

static CY_STATE_FLAG openFile(const char *path, FILE **fp, const char * _Mode)
{
    if(fp) *fp = NULL;
    if (!path || !fp) return CY_ERROR;
    
    *fp = fopen(path, _Mode);
    if (!*fp)
    {perror("fopen"); return CY_ERROR;}
    return CY_NORMAL;
}

static CY_STATE_FLAG sizeFile(FILE *fp, size_t *size)
{
    if (!fp || !size) return CY_ERROR;
    
    if (_fseeki64(fp, 0, SEEK_END) != 0)
    {perror("_fseeki64(END)"); return CY_ERROR;}

    __int64  fs = _ftelli64(fp);
    if (fs < 0) {perror("_ftelli64"); return CY_ERROR; }

    if ((uintmax_t) fs > (uintmax_t)SIZE_MAX) 
    {fprintf(stderr, "sizeFile: too large for size_t\n"); return CY_ERROR;}

    if (_fseeki64(fp, 0, SEEK_SET) != 0)
    {perror("_fseeki64(START)"); return CY_ERROR;}

    *size = (size_t) fs;
    return CY_NORMAL;
}

static CY_STATE_FLAG readFile(FILE *fp, uint8_t *dst, const size_t size) 
{
    if (!fp || (!dst && size)) return CY_ERROR;

    size_t offset = 0;
    while (offset < size) {
        size_t n = fread(dst + offset, 1, size - offset, fp);
        if (n == 0)
        {if (ferror(fp)) perror("fread"); else fprintf(stderr, "fread: unexpected EOF\n"); return CY_ERROR;}
        offset += n;
    }
    return CY_NORMAL;
}

static CY_STATE_FLAG writeFile(FILE *fp, const uint8_t *src, const size_t size) 
{
    if (!fp || (!src && size)) return CY_ERROR;

    size_t offset = 0;
    while (offset < size) {
        size_t n = fwrite(src + offset, 1, size - offset, fp);
        if (n == 0) 
        {perror("fwrite"); return CY_ERROR;}
        offset += n;
    }
    return CY_NORMAL;
}

static CY_STATE_FLAG closeFile(FILE *fp)
{
    if (!fp) return CY_NORMAL;
    if (fclose(fp) != 0) 
    {perror("fclose"); return CY_ERROR;}
    return CY_NORMAL;
}

static CY_STATE_FLAG mallocSpace(void **memp, size_t size)
{
    if (!memp) return CY_ERROR;
    *memp = NULL;

    if (size == 0) size = 1;

    *memp = malloc(size);
    if (!*memp) 
    {perror("malloc"); return CY_ERROR;}
    return CY_NORMAL;
}

static void freeFile(CY_String *file)
{
    if (!file) return;
    if (file->owner == CY_OWNED) free(file->str);
    file->str = NULL;
    file->owner = CY_NOT_OWNED;
    file->size = 0;
}

static CY_STATE_FLAG extractFile(const char *path, CY_String *buffer)
{
    // Startialize variables
    FILE *fp = NULL; *buffer = (CY_String) {NULL, 0, CY_NOT_OWNED};

    // opening the file
    if(openFile(path, &fp, "rb") == CY_ERROR) return CY_ERROR;

    size_t size = 0;

    // extracting the file size
    if(sizeFile(fp, &size) == CY_ERROR) {closeFile(fp); return CY_ERROR;}

    // mallocating buffer space
    uint8_t *memp;
    if(mallocSpace((void **) &memp, size + 1) == CY_ERROR) {closeFile(fp); return CY_ERROR;}

    // read the file
    if(readFile(fp, memp, size) == CY_ERROR) {closeFile(fp); free(memp); return CY_ERROR;}
    
    if(closeFile(fp) == CY_ERROR) {free(memp); return CY_ERROR;}

    *buffer = (CY_String){.str = memp, .size = size, .owner = CY_OWNED};
    buffer->str[size] = '\0';

    return CY_NORMAL;
}

/***************
 * END HELPERS *
 ***************/

/****************************** Cypher Functions ****************************/

static CY_STATE_FLAG cypherStart(const char *inpath, CY_String *file, uint8_t **buffer)
{

    if(extractFile(inpath, file) == CY_ERROR) return CY_ERROR;

    if(mallocSpace((void **) buffer, file->size + 1) == CY_ERROR) return CY_ERROR;

    (*buffer)[file->size] = '\0';
    
    return CY_NORMAL;
}

static CY_STATE_FLAG cypherFinish(const char *outpath, const CY_String file, const uint8_t *buffer)
{
    FILE *fp;

    if(openFile(outpath, &fp, "wb") == CY_ERROR) return CY_ERROR;

    if(writeFile(fp, buffer, file.size) == CY_ERROR) {closeFile(fp); return CY_ERROR;}

    if(closeFile(fp) == CY_ERROR) return CY_ERROR;

    return CY_NORMAL;
}


CY_STATE_FLAG cypher(const char *inpath, CY_KEY *key, const CY_FUNC cypherfunc, const CY_CRYPT crypt, const char *outpath)
{
    CY_String file; uint8_t *buffer;

    if(cypherStart(inpath, &file, &buffer) == CY_ERROR) {freeFile(&file); return CY_ERROR;}

    if(cypherfunc(file, crypt, &key, &buffer) == CY_ERROR) {free(buffer); freeFile(&file); return CY_ERROR;}

    if(cypherFinish(outpath, file, buffer) == CY_ERROR) {free(buffer); freeFile(&file); return CY_ERROR;}

    free(buffer); freeFile(&file); return CY_NORMAL;
}

CY_STATE_FLAG normal(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer)
{
    (void) key; (void) crypt;
    for (size_t i = 0; i < file.size; i++) (*buffer)[i] = file.str[i];
    return CY_NORMAL;
}


CY_STATE_FLAG caesar(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer)
{
    if(*key == NULL || (*key) == NULL) return CY_ERROR;
    uint64_t keyainv; uint64_t keya = (*key)->str[0]; uint64_t keyb = (*key)->str[1];
    if(EEA(keya, 26, &keyainv) == CY_ERROR) return CY_ERROR;
    keyb %= (uint8_t) 26;
    switch (crypt)
    {
    case CY_ENCRYPTION:
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
            (*buffer)[i] = c;
        } 
    break;
        
    case CY_DECRYPTION:
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
            (*buffer)[i] = c;
        } 
    break;
    
    default:
        fprintf(stderr, "No such parametre value monoalphabeticCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
        return CY_ERROR;
    }
    return CY_NORMAL;
}

CY_STATE_FLAG monoalphabetic(const CY_String file, const CY_CRYPT crypt, CY_KEY **key, uint8_t **buffer)
{
    if((*key)->str == NULL && (*key)->owner == CY_OWNED) return CY_ERROR;

        switch (crypt)
        {
            case CY_ENCRYPTION:
            if((*key)->owner == CY_NOT_OWNED)
            {
                if(mallocSpace((void **) &(*key)->str, 27) == CY_ERROR) return CY_ERROR;
                for (uint8_t i = 0; i < 26; i++) (*key)->str[i] = 'a' + i;
                for (uint8_t i = 0; i < 26; i++)
                {
                    uint8_t r; 
                    if(random_u8((uint8_t)(26 - i), &r) == CY_ERROR) return CY_ERROR;
                    char tmp = (*key)->str[i]; (*key)->str[i] = (*key)->str[r]; (*key)->str[r] = tmp;
                }
                (*key)->owner = CY_OWNED; (*key)->size = 26; (*key)->str[26] = '\0';
            }
            break;

            case CY_DECRYPTION:
            char chartable[] = "abcdefghijklmnopqrstuvwxyz";
            char freqchar[] = "etaoinshrdlcumwfgypbvkjxqz";
            if((*key)->owner == CY_NOT_OWNED)
            {
                if(mallocSpace((void **) &(*key)->str, 27) == CY_ERROR) return CY_ERROR;
                size_t numtable[26] = {0}; 
                for (size_t i = 0; i < file.size; i++)
                {
                    char c = file.str[i];
                    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                    {
                        char cas = 'a'; if((c >= 'A' && c <= 'Z')) cas = 'A';
                        numtable[c - cas]++;
                    }
                }
                
                for (size_t i = 0; i < 26; i++)
                {
                    size_t k = i;
                    for (size_t j = i + 1; j < 26; j++)
                        if(numtable[k] < numtable[j]) k = j;

                    size_t tmpnum = numtable[i]; numtable[i] = numtable[k]; numtable[k] = tmpnum;
                    char tmpchar = chartable[i]; chartable[i] = chartable[k]; chartable[k] = tmpchar;
                }
                (*key)->owner = CY_OWNED; (*key)->size = 26; (*key)->str[26] = '\0';
            }
            else
            {
                for (size_t i = 0; i < 26; i++) 
                {freqchar[i] = 'a' + i; chartable[i] = (*key)->str[i];}
            }
            for (size_t i = 0; i < 26; i++)
            (*key)->str[chartable[i] - 'a'] = freqchar[i];             
            break;
            
            default:
                fprintf(stderr, "No such parametre value monoalphabetic(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
                return CY_ERROR;
        }

    for (size_t i = 0; i < file.size; i++)
    {
       char c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            char cas = 'a'; if((c >= 'A' && c <= 'Z')) cas = 'A';
            c = (*key)->str[c - cas] - 'a' + cas;
        }
        (*buffer)[i] = c; 
    }

    return CY_NORMAL;
}