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




/******************************************************** 
 * 
 * 
 * 
 * 
 *                     Random Functions 
 *
 * 
 * 
 * 
 *********************************************************/




#define DECL_RANDOM_UNIFORM_UNSIGNED(T, name)                                                                           \
static CY_STATE_FLAG name(const T n, T *out)                                                                            \
{                                                                                                                       \
    if (!out || n == 0) return CY_ERROR;                                                                                \
                                                                                                                        \
    const T max = (T)~(T)0; /* 2^w - 1 */                                                                               \
                                                                                                                        \
    const T lim = (T)(max - (max % n)); /* multiple of n */                                                             \
                                                                                                                        \
    for (;;)                                                                                                            \
    {                                                                                                                   \
        NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, (ULONG)sizeof(T), BCRYPT_USE_SYSTEM_PREFERRED_RNG);            \
        if (st != 0) return CY_ERROR;                                                                                   \
        if (*out < lim)                                                                                                 \
        {*out = (T)(*out % n); return CY_NORMAL;}                                                                       \
    }                                                                                                                   \
}

DECL_RANDOM_UNIFORM_UNSIGNED(uint16_t, random_u16)
DECL_RANDOM_UNIFORM_UNSIGNED(__uint128_t, random_u128)




/******************************************************** 
 * 
 * 
 * 
 * 
 *                     Math Functions 
 *
 * 
 * 
 * 
 *********************************************************/




static uint64_t gcd(uint64_t a, uint64_t b)
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

static CY_STATE_FLAG EEA(uint64_t a, uint64_t n, uint64_t *out)
{
    if(gcd(a,n) != 1) 
    {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return CY_ERROR;}

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

static CY_STATE_FLAG MRA(const uint64_t n, CY_PRIMALITY_FLAG *out)
{
    *out = CY_INCONCLUSIVE;

    if(n < 3) 
    {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return CY_ERROR;}

    if(!(n % 2) || n == 3) return CY_NORMAL;

    __uint128_t a = n - 2;

    if(random_u128((uint64_t)(n - 1), &a) == CY_ERROR) return CY_ERROR;

    if(a < 2) a += 2; // 1 < a < n - 1

    __uint128_t a_q = 1, odd = n - 1;

    uint64_t power = 0;

    while (!(odd & 1))
    {
        odd >>= 1; // devid by 2
        power++;
    }

    while (odd) 
    { // fast multiplication using binary base shifting
        if (odd & 1) a_q = (a_q * a) % n;
        a = (a * a) % n;
        odd >>= 1;
    }

    if(a_q == 1) return CY_NORMAL;

    for (__uint128_t i = 0; i < power; i++)
    {
        if (a_q == (__uint128_t) (n - 1)) return CY_NORMAL;
        a_q = (a_q * a_q) % n;
    }

    *out = CY_COMPOSITE;

    return CY_NORMAL;
}

static CY_STATE_FLAG EMRA(const uint64_t n, const uint64_t prob, CY_PRIMALITY_FLAG *out)
{
    if (MRA(n,out) == CY_ERROR) return CY_ERROR;

    for (__uint128_t i = 0; i < prob; i++) MRA(n,out);

    return CY_NORMAL;  
}

static CY_STATE_FLAG CRT(const CY_Residu64 a[], const uint64_t size, uint64_t *out)
{
    *out = 0;
    if(!size) 
    {fprintf(stderr, "Can't have null size"); return CY_ERROR;}

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

        if(EEA((uint64_t) Mi, a[i].mod, &Mi_inv) == CY_ERROR) return CY_ERROR;
        
        __uint128_t t = (Mi * Mi_inv) % M;
        t = (t * (a[i].value % a[i].mod)) % M;
        A = (uint64_t) ((t + A) % M);
    }

    *out = A % M;

    return CY_NORMAL;
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                     File Functions 
 *
 * 
 * 
 * 
 *********************************************************/




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
    if (fs < 0) 
    {perror("_ftelli64"); return CY_ERROR;}

    if ((uintmax_t) fs > (uintmax_t)SIZE_MAX) 
    {fprintf(stderr, "sizeFile: too large for size_t\n"); return CY_ERROR;}

    if (_fseeki64(fp, 0, SEEK_SET) != 0)
    {perror("_fseeki64(START)"); return CY_ERROR;}

    *size = (size_t) fs;
    return CY_NORMAL;
}

static CY_STATE_FLAG readFile(FILE *fp, void *dst, const size_t size) 
{
    if (!fp || (!dst && size)) return CY_ERROR;

    size_t offset = 0;
    while (offset < size) 
    {
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
    while (offset < size) 
    {
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

static CY_STATE_FLAG mallocSpace(void **memp, const size_t size)
{
    if (!memp) return CY_ERROR;

    *memp = NULL;
    *memp = malloc(size ? size : 1);

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
    FILE *fp = NULL; *buffer = (CY_String) {NULL, 0, CY_NOT_OWNED};

    if(openFile(path, &fp, "rb") == CY_ERROR) return CY_ERROR;

    size_t size = 0;

    if(sizeFile(fp, &size) == CY_ERROR) 
    {closeFile(fp); return CY_ERROR;}

    uint8_t *memp;
    if(mallocSpace((void **) &memp, size + 1) == CY_ERROR) 
    {closeFile(fp); return CY_ERROR;}

    if(readFile(fp, memp, size) == CY_ERROR) 
    {closeFile(fp); free(memp); return CY_ERROR;}
    
    if(closeFile(fp) == CY_ERROR)
    {free(memp); return CY_ERROR;}

    *buffer = (CY_String){.str = memp, .size = size, .owner = CY_OWNED};
    buffer->str[size] = '\0';

    return CY_NORMAL;
}




/***************
 * END HELPERS *
 ***************/




/******************************************************** 
 * 
 * 
 * 
 * 
 *                      Key Functions 
 *
 * 
 * 
 * 
 *********************************************************/




CY_STATE_FLAG CY_GENERATE_key(const uint8_t start, const uint8_t end, CY_KEY *key)
{
    if(key->owner == CY_OWNED || key->str != NULL || key->size != 0) 
    {fprintf(stderr, "CY_GENERATE_key(-> KEY_ERROR: the key is (owned, not empty or size is not 0) <-)\n"); return CY_ERROR;} 

    if(start > end)
    {fprintf(stderr, "CY_GENERATE_key(-> SIZE_ERROR: negative size not allowed (end must be greater than start) <-)\n"); return CY_ERROR;}

    if(mallocSpace((void **) &key->str, (end - start + 1) * sizeof(uint8_t)) == CY_ERROR) return CY_ERROR;

    key->owner = CY_OWNED; key->size = end - start + 1; 
    
    for (uint16_t i = 0; i < key->size; i++) key->str[i] = (uint8_t) (i + start);

    return CY_NORMAL;
}

CY_STATE_FLAG CY_SHIFT_key(const uint8_t start, const uint8_t end, const CY_KEY *key, CY_String buffer)
{
    if(key->owner == CY_NOT_OWNED || key->str == NULL || key->size != 2) 
    {fprintf(stderr, "CY_SHIFT_key(-> KEY_ERROR: the key is (not owned, empty or size (end - start + 1) is greater then the key size) <-)\n"); return CY_ERROR;}

    if(start > end)
    {fprintf(stderr, "CY_SHIFT_key(-> SIZE_ERROR: negative size not allowed (end must be greater than start) <-)\n"); return CY_ERROR;}

    uint8_t keya = key->str[0], keyb = key->str[1];
    uint16_t mod = (end - start + 1);

    keya %= mod; keyb %= mod;

    if(gcd(keya, mod) != 1)
    {fprintf(stderr, "gcd(%"PRIu8",%"PRIu16") is different than 1\n", keya, mod); return CY_ERROR;}

    for (size_t i = 0; i < buffer.size; i++)
    {
        char c = buffer.str[i];
        c = (c >= start && c <= end) ? (char) ((keya * (c - start) + keyb) % mod + start) : c;
        buffer.str[i] = c;
    }
    return CY_NORMAL;
}

CY_STATE_FLAG CY_INVSHIFT_key(const uint8_t start, const uint8_t end, const CY_KEY *key, CY_String buffer)
{
    if(key->owner == CY_NOT_OWNED || key->str == NULL || key->size != 2) 
    {fprintf(stderr, "CY_SHIFT_key(-> KEY_ERROR: the key is (not owned, empty or size (end - start + 1) is greater then the key size) <-)\n"); return CY_ERROR;}

    if(buffer.owner == CY_NOT_OWNED || buffer.str == NULL || buffer.size == 0) 
    {fprintf(stderr, "CY_SHIFT_key(-> BUFFER_ERROR: the buffer is (not owned, empty or size is 0 <-)\n"); return CY_ERROR;}

    if(start > end)
    {fprintf(stderr, "CY_SHIFT_key(-> SIZE_ERROR: negative size not allowed (end must be greater than start) <-)\n"); return CY_ERROR;}

    uint16_t mod = (end - start + 1);
    uint64_t keya, keyb = key->str[1];

    if(EEA(key->str[0], mod, &keya) == CY_ERROR) return CY_ERROR;

    for (size_t i = 0; i < buffer.size; i++)
    {
        char c = buffer.str[i];
        buffer.str[i] = (c >= start && c <= end) ? (uint8_t) ((keya * (mod + (c - start) - keyb % mod)) % mod + start) : c;
    }
    return CY_NORMAL;
}

CY_STATE_FLAG CY_RAND_key(const uint8_t start, const uint8_t end, CY_KEY *key)
{
    if(key->owner == CY_NOT_OWNED || key->str == NULL || key->size < (size_t) (end - start + 1)) 
    {fprintf(stderr, "CY_RAND_key(-> KEY_ERROR: the key is (not owned, empty or size (end - start + 1) is greater then the key size) <-)\n"); return CY_ERROR;}

    if(start > end)
    {fprintf(stderr, "CY_RAND_key(-> SIZE_ERROR: negative size not allowed (end must be greater than start) <-)\n"); return CY_ERROR;}

    for (uint16_t i = start; i <= end; i++)
    {
        uint16_t count = (uint16_t) (end - i + 1);
        uint16_t r; if(random_u16(count, &r) == CY_ERROR) {return CY_ERROR;} r+= (i - start);
        uint8_t tmp = key->str[(uint8_t) (i - start)]; key->str[(uint8_t) (i - start)] = key->str[(uint8_t) r]; key->str[(uint8_t) r] = tmp;
    }

    return CY_NORMAL;
}

CY_STATE_FLAG CY_INVERSE_key(const uint8_t start, const uint8_t end, CY_KEY key, CY_KEY keymap, CY_KEY *invkey)
{
    if(key.owner == CY_NOT_OWNED || key.str == NULL || key.size < (size_t) (end - start + 1)) 
    {fprintf(stderr, "CY_INVERSE_key(-> KEY_ERROR: the key is (not owned, empty or size (end - start + 1) is greater then the key size)<-)\n"); return CY_ERROR;}

    if(keymap.owner == CY_NOT_OWNED || keymap.str == NULL || keymap.size != key.size) 
    {fprintf(stderr, "CY_INVERSE_key(-> KEY_ERROR: the keymap is (not owned, empty or size is diffrent then the key <-)\n"); return CY_ERROR;}

    if(mallocSpace((void **) &invkey->str, key.size * sizeof(uint8_t)) == CY_ERROR) return CY_ERROR; 

    invkey->owner = CY_OWNED; invkey->size = key.size;

    for (uint16_t i = 0; i < (uint16_t) invkey->size; i++) invkey->str[key.str[i] - start] = keymap.str[i];

    return CY_NORMAL;
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                    ASCII Functions 
 *
 * 
 * 
 * 
 *********************************************************/




static CY_STATE_FLAG CY_NORMAL_EASCII(const uint8_t start, const uint8_t end, CY_String file, CY_KEY *key, uint8_t **buffer)
{   
    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        (*buffer)[i] = (c >= start && c <= end) ? key->str[c - start] : c;
    }
    return CY_NORMAL;
}

static CY_STATE_FLAG CY_FREQUENT_EASCII(const uint8_t start, const uint8_t end, CY_String file, size_t *numtable)
{   
    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if(c >= start && c <= end) numtable[c - start]++;
    }
    return CY_NORMAL;
}

static CY_STATE_FLAG CY_NORMAL_ABC(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(CY_NORMAL_EASCII('a', 'z', file, key, buffer) == CY_ERROR) return CY_ERROR;

    for (size_t i = 0; i < key->size; i++) key->str[i] = key->str[i] - 'a' + 'A';

    if(CY_NORMAL_EASCII('A', 'Z', (CY_String) {.owner=file.owner, .size=file.size, .str=(*buffer)}, key, buffer) == CY_ERROR) return CY_ERROR;

    for (size_t i = 0; i < key->size; i++) key->str[i] = key->str[i] - 'A' + 'a';

    return CY_NORMAL;
}

static CY_STATE_FLAG CY_FREQUENT_ABC(const CY_String file, size_t *numtable)
{
    if(CY_FREQUENT_EASCII('a', 'z', file, numtable) == CY_ERROR) return CY_ERROR;

    return CY_FREQUENT_EASCII('A', 'Z', file, numtable);
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                    Cypher Functions 
 *
 * 
 * 
 * 
 *********************************************************/




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

CY_STATE_FLAG cypher(const char *inpath, CY_KEY *key, const CY_FUNC cypherfunc, const char *outpath)
{
    CY_String file; uint8_t *buffer;

    if(cypherStart(inpath, &file, &buffer) == CY_ERROR) {freeFile(&file); return CY_ERROR;}

    if(cypherfunc(file, key, &buffer) == CY_ERROR) {free(buffer); freeFile(&file); return CY_ERROR;}

    if(cypherFinish(outpath, file, buffer) == CY_ERROR) {free(buffer); freeFile(&file); return CY_ERROR;}

    free(buffer); freeFile(&file);

    return CY_NORMAL;
}

CY_STATE_FLAG CY_encryption_caesar(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    for (size_t i = 0; i < file.size; i++) (*buffer)[i] = file.str[i];

    if(CY_SHIFT_key('a', 'z', key, (CY_String) {.owner=file.owner, .size=file.size, .str=(*buffer)}) == CY_ERROR) return CY_ERROR;

    return CY_SHIFT_key('A', 'Z', key, (CY_String) {.owner=file.owner, .size=file.size, .str=(*buffer)});
}

CY_STATE_FLAG CY_decryption_caesar(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    for (size_t i = 0; i < file.size; i++) (*buffer)[i] = file.str[i];

    if(CY_INVSHIFT_key('a', 'z', key, (CY_String) {.owner=file.owner, .size=file.size, .str=(*buffer)}) == CY_ERROR) return CY_ERROR;
    
    return CY_INVSHIFT_key('A', 'Z', key, (CY_String) {.owner=file.owner, .size=file.size, .str=(*buffer)});
}

CY_STATE_FLAG CY_encryption_monoalphabetic(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(key->owner == CY_NOT_OWNED)
    {
        if(CY_GENERATE_key('a', 'z', key) == CY_ERROR) return CY_ERROR;

        if(CY_RAND_key('a', 'z', key) == CY_ERROR) return CY_ERROR;
    }

    if(key->str == NULL) 
    {fprintf(stderr, "CY_encryption_monoalphabetic(-> KEY_ERROR: the key is owned but its null <-)\n"); return CY_ERROR;}

    return CY_NORMAL_ABC(file, key, buffer);
}

CY_STATE_FLAG CY_decryption_monoalphabetic(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    uint8_t keymap[] = "abcdefghijklmnopqrstuvwxyz";
    
    if(CY_INVERSE_key('a', 'z', *key, (CY_KEY) {.owner=CY_OWNED, .size=26, .str=keymap}, key) == CY_ERROR) return CY_ERROR;
    
    if(CY_NORMAL_ABC(file, key, buffer) == CY_ERROR) return CY_ERROR;

    return CY_INVERSE_key('a', 'z', *key, (CY_KEY) {.owner=CY_OWNED, .size=26, .str=keymap}, key);
}

CY_STATE_FLAG CY_crack_monoalphabetic(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(key->owner == CY_OWNED || key->str != NULL || key->size != 0) 
    {fprintf(stderr, "CY_GENERATE_key(-> KEY_ERROR: the key is (owned, not empty or size is not 0) <-)\n"); return CY_ERROR;}

    *key = (CY_KEY) {.owner=CY_OWNED, .size=26, .str=NULL};
    if(mallocSpace((void **) &key->str, 26) == CY_ERROR) return CY_ERROR;

    size_t numtable[26] = {0};
    uint8_t freqchar[] = "etaoinshrdlcumwfgypbvkjxqz";
    uint8_t chartable[] = "abcdefghijklmnopqrstuvwxyz";
    
    if (CY_FREQUENT_ABC(file, numtable) == CY_ERROR) return CY_ERROR;

    for (size_t i = 0; i < 26; i++)
    {
        size_t k = i;

        for (size_t j = i + 1; j < 26; j++) if(numtable[k] < numtable[j]) k = j;

        size_t tmpnum = numtable[i]; numtable[i] = numtable[k]; numtable[k] = tmpnum;

        char tmpchar = chartable[i]; chartable[i] = chartable[k]; chartable[k] = tmpchar;
    }
    
    if(CY_INVERSE_key('a', 'z', (CY_KEY) {.owner=CY_OWNED, .size=26, .str=chartable}, (CY_KEY) {.owner=CY_OWNED, .size=26, .str=freqchar}, key) == CY_ERROR) return CY_ERROR;
    
    return CY_NORMAL_ABC(file, key, buffer);
}

CY_STATE_FLAG CY_encryption_EASCII(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(key->owner == CY_NOT_OWNED)
    {
        if(CY_GENERATE_key(0, 255, key) == CY_ERROR) return CY_ERROR;

        if(CY_RAND_key(0, 255, key) == CY_ERROR) return CY_ERROR;
    }

    if(key->str == NULL) 
    {fprintf(stderr, "CY_encryption_EASCII(-> KEY_ERROR: the key is owned but its null <-)\n"); return CY_ERROR;}

    return CY_NORMAL_EASCII(0, 255, file, key, buffer);
}

CY_STATE_FLAG CY_decryption_EASCII(const CY_String file, CY_KEY *key, uint8_t **buffer)
{
    uint8_t keymap[256];
    for (uint16_t i = 0; i < 256; i++) keymap[i] = (uint8_t) i;
    
    if(CY_INVERSE_key(0, 255, *key, (CY_KEY) {.owner=CY_OWNED, .size=256, .str=keymap}, key) == CY_ERROR) return CY_ERROR;
    
    if(CY_NORMAL_EASCII(0, 255,file, key, buffer) == CY_ERROR) return CY_ERROR;

    return CY_INVERSE_key(0, 255, *key, (CY_KEY) {.owner=CY_OWNED, .size=256, .str=keymap}, key);
}