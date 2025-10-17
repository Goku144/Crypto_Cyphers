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



#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")

  // Keep original call/semantics (NTSTATUS == 0 on success)
  static inline int cy_random_bytes(void *p, size_t n) {
      return BCryptGenRandom(NULL, (PUCHAR)p, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  }

  #define CY_FSEEK  _fseeki64
  #define CY_FTELL  _ftelli64

#else
  // Linux / POSIX
  #define _FILE_OFFSET_BITS 64   // make off_t 64-bit for fseeko/ftello
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>

  // If Linux with getrandom:
  #if defined(__linux__)
    #include <sys/random.h>
    static inline int cy_random_bytes(void *p, size_t n) {
        // Return 0 on success to match BCryptGenRandom’s "0 == success" convention
        ssize_t r = getrandom(p, n, 0);
        return (r == (ssize_t)n) ? 0 : -1;
    }
  #else
    // Fallback: /dev/urandom
    static inline int cy_random_bytes(void *p, size_t n) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) return -1;
        size_t off = 0;
        while (off < n) {
            ssize_t r = read(fd, (char*)p + off, n - off);
            if (r <= 0) {
                if (errno == EINTR) continue;
                close(fd);
                return -1;
            }
            off += (size_t)r;
        }
        close(fd);
        return 0;
    }
  #endif

  // Provide POSIX large-file functions under your existing names
  #include <stdio.h>
  #define _fseeki64(stream, off, whence) fseeko((stream), (off_t)(off), (whence))
  #define _ftelli64(stream)              ftello((stream))
  typedef unsigned char *PUCHAR;  // matches Windows PUCHAR
  typedef unsigned long  ULONG;   // good enough for your (ULONG) casts

  // Make the code that expects NTSTATUS + BCrypt style compile unchanged
  typedef int NTSTATUS;
  #define BCRYPT_USE_SYSTEM_PREFERRED_RNG 0U
#endif

// Unify the RNG call so your existing code compiles unchanged
// (NTSTATUS == 0 on success in both branches)
#define BCryptGenRandom(h, p, l, f) cy_random_bytes((p), (size_t)(l))
// -----------------------------------------




/***************** 
 * START HELPERS *
 *****************/




 /******************************************************** 
 * 
 * 
 * 
 * 
 *                     ERROR Functions 
 *
 * 
 * 
 * 
 *********************************************************/




static CY_STATE_FLAG cy_err_manager(const CY_STATE_FLAG e, const char *funcname, const char *msg){
    switch(e)
    {
    case CY_OK: return CY_OK;
    case CY_ERR: return CY_ERR;
    case CY_ERR_ARG: fprintf(stderr,"%s(-> ERROR(-> invalid argument <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_STATE: fprintf(stderr,"%s(-> ERROR(-> bad state/order <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_UNSUPPORTED: fprintf(stderr,"%s(-> ERROR(-> unsupported <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_NOT_IMPL: fprintf(stderr,"%s(-> ERROR(-> not implemented <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_INTERNAL: fprintf(stderr,"%s(-> ERROR(-> internal error <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_OOM: fprintf(stderr,"%s(-> ERROR(-> out of memory <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_SPACE: fprintf(stderr,"%s(-> ERROR(-> insufficient output buffer <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_SIZE: fprintf(stderr,"%s(-> ERROR(-> invalid size <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_OVERLAP: fprintf(stderr,"%s(-> ERROR(-> buffer overlap <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_IO: fprintf(stderr,"%s(-> ERROR(-> I/O error <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_EOF: fprintf(stderr,"%s(-> ERROR(-> unexpected EOF <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_OPEN: fprintf(stderr,"%s(-> ERROR(-> open failed <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_CLOSE: fprintf(stderr,"%s(-> ERROR(-> close failed <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_FORMAT: fprintf(stderr,"%s(-> ERROR(-> format parse error <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_VALUE: fprintf(stderr,"%s(-> ERROR(-> invalid value <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_RANGE: fprintf(stderr,"%s(-> ERROR(-> out of range <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_RNG: fprintf(stderr,"%s(-> ERROR(-> rng failure <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_KEY: fprintf(stderr,"%s(-> ERROR(-> key error <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_KEY_SIZE: fprintf(stderr,"%s(-> ERROR(-> key size invalid <-)%s <-)\n", funcname, msg); return CY_ERR;
    case CY_ERR_KEY_VALUE: fprintf(stderr,"%s(-> ERROR(-> key content invalid <-)%s <-)\n", funcname, msg); return CY_ERR;
    default: fprintf(stderr,"%s(-> ERROR(-> unknown error <-)%s <-)\n", funcname, msg); return CY_ERR;
    }
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                 General Helper Functions 
 *
 * 
 * 
 * 
 *********************************************************/




// is used in the playfair cypher
static void find_matrix_pos(const uint8_t *fairkey, const uint8_t val, uint8_t *posx, uint8_t *posy)
{
    for (uint8_t i = 0; i < 16; i++)
    {   
        for (uint8_t j = 0; j < 16; j++)
        {
            if(val == fairkey[i*16 + j]) {*posx = i; *posy = j;}
        }
    }
}




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
    if (!out || n == 0) return CY_ERR;                                                                                  \
                                                                                                                        \
    const T max = (T)~(T)0; /* 2^w - 1 */                                                                               \
                                                                                                                        \
    const T lim = (T)(max - (max % n)); /* multiple of n */                                                             \
                                                                                                                        \
    for (;;)                                                                                                            \
    {                                                                                                                   \
        NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, (ULONG)sizeof(T), BCRYPT_USE_SYSTEM_PREFERRED_RNG);            \
        if (st != 0) return CY_ERR;                                                                                     \
        if (*out < lim)                                                                                                 \
        {*out = (T)(*out % n); return CY_OK;}                                                                           \
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
    {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return CY_ERR;}

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

    return CY_OK;
}

static CY_STATE_FLAG MRA(const uint64_t n, CY_PRIMALITY_FLAG *out)
{
    *out = CY_INCONCLUSIVE;

    if(n < 3) 
    {fprintf(stderr, "%"PRIu64" < 3, decomposition condition unsatisfied.\n", n); return CY_ERR;}

    if(!(n % 2) || n == 3) return CY_OK;

    __uint128_t a = n - 2;

    if(random_u128((uint64_t)(n - 1), &a) == CY_ERR) return CY_ERR;

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

    if(a_q == 1) return CY_OK;

    for (__uint128_t i = 0; i < power; i++)
    {
        if (a_q == (__uint128_t) (n - 1)) return CY_OK;
        a_q = (a_q * a_q) % n;
    }

    *out = CY_COMPOSITE;

    return CY_OK;
}

static CY_STATE_FLAG EMRA(const uint64_t n, const uint64_t prob, CY_PRIMALITY_FLAG *out)
{
    if (MRA(n,out) == CY_ERR) return CY_ERR;

    for (__uint128_t i = 0; i < prob; i++) MRA(n,out);

    return CY_OK;  
}

static CY_STATE_FLAG CRT(const CY_Residu64 a[], const uint64_t size, uint64_t *out)
{
    *out = 0;
    if(!size) 
    {fprintf(stderr, "Can't have null size"); return CY_ERR;}

    uint64_t M = 1;

    uint64_t A = 0;

    for (uint64_t i = 0; i < size; i++)
    {
        if(!a[i].mod)
        {fprintf(stderr, "Can't have null modulos"); return CY_ERR;}

        uint64_t gcd_v = gcd(a[i].mod,M);

        if(gcd_v != 1)
        {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") = %"PRIu64" CRT condition not meet\n", a[i].mod, (uint64_t) M, gcd_v); return CY_ERR;}

        // check if M has overflowed
        if(M > UINT64_MAX / a[i].mod) return CY_ERR;
        M *= a[i].mod;    
    }

    M = M ? M : UINT64_MAX;

    for (uint64_t i = 0; i < size; i++)
    {
        uint64_t Mi = M / a[i].mod;
        uint64_t Mi_inv = 0;

        if(EEA((uint64_t) Mi, a[i].mod, &Mi_inv) == CY_ERR) return CY_ERR;
        
        __uint128_t t = (Mi * Mi_inv) % M;
        t = (t * (a[i].value % a[i].mod)) % M;
        A = (uint64_t) ((t + A) % M);
    }

    *out = A % M;

    return CY_OK;
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




static CY_STATE_FLAG open_file(FILE **fp, const char *mode, const char *path)
{
    if(!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
    if(!mode || !*mode) return cy_err_manager(CY_ERR_ARG, __func__, ": mode is NULL/empty");
    if(!path || !*path) return cy_err_manager(CY_ERR_ARG, __func__, ": path is NULL/empty");

    *fp = NULL; *fp = fopen(path, mode);
    if(!*fp) {perror("fopen"); return cy_err_manager(CY_ERR_OPEN, __func__, "");}

    return CY_OK;
}

static CY_STATE_FLAG size_file(FILE *fp, size_t *size)
{
    if (!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
    if(!size) return cy_err_manager(CY_ERR_ARG, __func__, ": size is NULL");
    
    if (_fseeki64(fp, 0, SEEK_END) != 0)
    {perror("_fseeki64(END)"); return cy_err_manager(CY_ERR_IO, __func__, ": seek end failed");}

    __int64_t  fs = _ftelli64(fp);
    if (fs < 0) 
    {perror("_ftelli64"); return cy_err_manager(CY_ERR_IO, __func__, ": tell failed");}

    if ((uintmax_t) fs > (uintmax_t) SIZE_MAX) return cy_err_manager(CY_ERR_SIZE, __func__, "file too large for size_t");

    if (_fseeki64(fp, 0, SEEK_SET) != 0)
    {perror("_fseeki64(SET)"); return cy_err_manager(CY_ERR_IO, __func__, "rewind failed");}

    *size = (size_t) fs;
    return CY_OK;
}

static CY_STATE_FLAG malloc_space(void **memp, const size_t size)
{
    if (!memp) return cy_err_manager(CY_ERR_ARG, __func__, ": memp is NULL");
    if (!size) return cy_err_manager(CY_ERR_ARG, __func__, ": size is 0");

    *memp = NULL;
    *memp = malloc(size);

    if (!*memp) 
    {perror("malloc"); return cy_err_manager(CY_ERR_OOM, __func__, ": malloc faild to allocate space");}

    return CY_OK;
}

static CY_STATE_FLAG realloc_space(void **memp, const size_t size)
{
        if (!memp) return cy_err_manager(CY_ERR_ARG, __func__, ": memp is NULL");
    if (!size) return cy_err_manager(CY_ERR_ARG, __func__, ": size is 0");

    *memp = realloc(*memp, size);

    if (!*memp) 
    {perror("realloc"); return cy_err_manager(CY_ERR_OOM, __func__, ": realloc faild to reallocate space");}

    return CY_OK;
}

static CY_STATE_FLAG free_space(CY_String *file)
{
    if (!file || !file->str) return cy_err_manager(CY_ERR_ARG, __func__, ": file/file->str is NULL");

    if (file->owner == CY_OWNED) free(file->str);

    file->str = NULL; file->owner = CY_NOT_OWNED; file->size = 0;

    return CY_OK;
}

static CY_STATE_FLAG read_file(FILE *fp, void *dst, const size_t size)
{
    if (!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
    if (!dst) return cy_err_manager(CY_ERR_ARG, __func__, ": dst is NULL");
    if (!size) return CY_OK;

    uint8_t *p = (uint8_t *)dst;
    size_t offset = 0;

    while (offset < size) 
    {
        size_t n = fread(p + offset, 1, size - offset, fp);

        if (n == 0)
        {
            if (ferror(fp)) 
            {perror("fread"); return cy_err_manager(CY_ERR_IO, __func__, ": fread faild to read the full file");} 
            
            return cy_err_manager(CY_ERR_EOF, __func__, ": fread stopped unexpectedly");
        }

        offset += n;
    }
    return CY_OK;
}

static CY_STATE_FLAG write_file(FILE *fp, const void *src, const size_t size)
{
    if (!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
    if (!src) return cy_err_manager(CY_ERR_ARG, __func__, ": src is NULL");
    if (!size) return CY_OK;

    uint8_t *p = (uint8_t *)src;
    size_t offset = 0;

    while (offset < size) 
    {
        size_t n = fwrite(p + offset, 1, size - offset, fp);
        if (n == 0) 
        {
            if (ferror(fp)) 
            {perror("fwrite"); return cy_err_manager(CY_ERR_IO, __func__, ": fwrite faild to write the full file");} 

            return cy_err_manager(CY_ERR_INTERNAL, __func__, ": fwrite made no progress");
        }
        offset += n;
    }
    return CY_OK;
}

static CY_STATE_FLAG close_file(FILE *fp)
{
    if (!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");

    if (fclose(fp) != 0) 
    {perror("fclose"); return cy_err_manager(CY_ERR_CLOSE, __func__, ": fclose faild to close fp");}

    return CY_OK;
}

static CY_STATE_FLAG extract_file(const char *path, CY_String *buffer)
{
    FILE *fp = NULL; *buffer = (CY_String) {NULL, 0, CY_NOT_OWNED};

    if(open_file(&fp, "rb", path) == CY_ERR) return CY_ERR;

    size_t size = 0;

    if(size_file(fp, &size) == CY_ERR) 
    {close_file(fp); return CY_ERR;}

    uint8_t *memp;
    if(malloc_space((void **) &memp, size + 1) == CY_ERR) 
    {close_file(fp); return CY_ERR;}

    if(read_file(fp, memp, size) == CY_ERR) 
    {close_file(fp); free(memp); return CY_ERR;}
    
    if(close_file(fp) == CY_ERR)
    {free(memp); return CY_ERR;}

    *buffer = (CY_String){.str = memp, .size = size, .owner = CY_OWNED};
    buffer->str[size] = '\0';

    return CY_OK;
}




/***************
 * END HELPERS *
 ***************/




/******************************************************** 
 * 
 * 
 * 
 * 
 *                    check Functions 
 *
 * 
 * 
 * 
 *********************************************************/




static CY_STATE_FLAG cy_key_linear_check_direct_generate(const uint8_t start, const uint8_t end, const CY_KEY *key)
{
    if(start > end) return cy_err_manager(CY_ERR_ARG, __func__, ": start is greater than end");
    if(key->str || key->owner == CY_OWNED) return cy_err_manager(CY_ERR_ARG, __func__, "key is owned");
    return CY_OK;
}

static CY_STATE_FLAG cy_key_linear_check_rand_generated(const uint8_t start, const uint8_t end, const CY_KEY *key)
{
    if(start > end) return cy_err_manager(CY_ERR_ARG, __func__, ": start is greater than end");
    if(!key->str || key->owner == CY_NOT_OWNED || !key->size) return cy_err_manager(CY_ERR_ARG, __func__, "key is not owned");
    uint16_t size = (end - start + 1);
    if(key->size < size) return cy_err_manager(CY_ERR_KEY_SIZE, __func__, ": key size is less then the start to end size");
    return CY_OK;
}

static CY_STATE_FLAG cy_key_linear_check_inverse_generated(const uint8_t start, const uint8_t end, const CY_KEY mapkey, const CY_KEY key, const CY_KEY *invkey)
{
    if(start > end) return cy_err_manager(CY_ERR_ARG, __func__, ": start is greater than end");
    if(!mapkey.str || mapkey.owner == CY_NOT_OWNED || !mapkey.size) return cy_err_manager(CY_ERR_ARG, __func__, ": mapkey is NULL, not owned or size is 0");
    if(!key.str || key.owner == CY_NOT_OWNED || !key.size) return cy_err_manager(CY_ERR_ARG, __func__, ": key is NULL, not owned or size is 0");
    if(invkey->str || invkey->owner == CY_OWNED || invkey->size) return cy_err_manager(CY_ERR_ARG, __func__, ": invkey is not NULL, owned, or size is not 0");
    uint16_t size = (end - start + 1);
    if(key.size < size) return cy_err_manager(CY_ERR_KEY_SIZE, __func__, ": key size is less than the start to end size");
    if(key.size != mapkey.size) return cy_err_manager(CY_ERR_KEY_SIZE, __func__, ": key and mapkey have deffrent sizes");
    for(uint16_t i = 0; i < size; i++) if(key.str[i] < start) return cy_err_manager(CY_ERR_KEY, __func__, ": one of the key value is smaller than start value");
    return CY_OK;
}

static CY_STATE_FLAG cy_check_caesar(const CY_KEY key)
{
    if(!key.str || key.owner == CY_NOT_OWNED || key.size != 2) return cy_err_manager(CY_ERR_ARG, __func__, ": key is NULL, not owned or size is not 2");
    if(gcd(key.str[0], 26) != 1) return cy_err_manager(CY_ERR_KEY, __func__, ": gcd of a and 26 is not 1 (key = (->a<-,b))");
    return CY_OK;
}

static CY_STATE_FLAG cy_check_monoalpahbetic(const CY_KEY key)
{
    if(!key.str || key.owner == CY_NOT_OWNED || key.size != 26) return cy_err_manager(CY_ERR_ARG, __func__, ": key is NULL, not owned or size is not 26");
    for (uint16_t i = 0; i < 26; i++) 
        if(!((key.str[i] >= 'a' && key.str[i] <= 'z') || (key.str[i] >= 'A' && key.str[i] <= 'Z')))
            return cy_err_manager(CY_ERR_KEY, __func__, ": key has value that are not letter");
    return CY_OK;
}

static CY_STATE_FLAG cy_check_eascii(const CY_KEY key)
{
    if(!key.str || key.owner == CY_NOT_OWNED || key.size != 256) return cy_err_manager(CY_ERR_ARG, __func__, ": key is NULL, not owned or size is not 256");
    return CY_OK;
}

static CY_STATE_FLAG cy_check_playfaire(const CY_KEY key)
{
    if(!key.str || key.owner == CY_NOT_OWNED || key.size != 256) return cy_err_manager(CY_ERR_ARG, __func__, ": key is NULL, not owned or size is not 256");
    return CY_OK;
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                  linear Key Functions 
 *
 * 
 * 
 * 
 *********************************************************/




CY_STATE_FLAG cy_key_linear_direct_generate(const uint8_t start, const uint8_t end, CY_KEY *key)
{
    if(cy_key_linear_check_direct_generate(start, end, key) == CY_ERR) return CY_ERR;
    uint16_t size = (end - start + 1);
    if(malloc_space((void **) &key->str, size) == CY_ERR) return CY_ERR;
    for (uint16_t i = 0; i < size; i++) key->str[i] =  (i + start);
    key->owner = CY_OWNED; key->size = size;
    return CY_OK;
}

CY_STATE_FLAG CY_key_linear_rand_generated(const uint8_t start, const uint8_t end, CY_KEY *key)
{
    if(cy_key_linear_check_rand_generated(start, end, key) == CY_ERR) return CY_ERR;
    for (uint16_t i = start; i <= end; i++)
    {
        uint16_t count = (uint16_t) (end - i + 1);
        uint16_t r; if(random_u16(count, &r) == CY_ERR) {return CY_ERR;} r+= (i - start);
        uint8_t tmp = key->str[(uint8_t) (i - start)]; key->str[(uint8_t) (i - start)] = key->str[(uint8_t) r]; key->str[(uint8_t) r] = tmp;
    }
    key->owner = CY_OWNED; key->size = (end - start + 1);
    return CY_OK;
}

CY_STATE_FLAG cy_key_linear_inverse_generated(const uint8_t start, const uint8_t end, const CY_KEY mapkey, const CY_KEY key, CY_KEY *invkey)
{
    if(cy_key_linear_check_inverse_generated(start, end , mapkey, key, invkey) == CY_ERR) return CY_ERR;
    uint16_t size = (end - start + 1);
    if(malloc_space((void **)  &invkey->str, size) == CY_ERR) return CY_ERR;
    for (uint16_t i = 0; i < size; i++) invkey->str[key.str[i] - start] = mapkey.str[i];
    invkey->owner = CY_OWNED; invkey->size = size;
    return CY_OK;
}

CY_STATE_FLAG cy_key_import(const char *inpath, CY_KEY *key)
{
    return extract_file(inpath, key);   
}

CY_STATE_FLAG cy_key_export(const CY_KEY key, const char *outpath)
{
    FILE *fp;

    if(open_file(&fp, "wb", outpath) == CY_ERR) return CY_ERR;

    if(write_file(fp, key.str, key.size) == CY_ERR) {close_file(fp); return CY_ERR;}

    return close_file(fp);   
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
    if(extract_file(inpath, file) == CY_ERR) return CY_ERR;

    if(malloc_space((void **) buffer, file->size + 1) == CY_ERR) return CY_ERR;

    (*buffer)[file->size] = '\0';
    
    return CY_OK;
}

static CY_STATE_FLAG cypherFinish(const char *outpath, const CY_String file, const uint8_t *buffer)
{
    FILE *fp;

    if(open_file(&fp, "wb", outpath) == CY_ERR) return CY_ERR;

    if(write_file(fp, buffer, file.size) == CY_ERR) {close_file(fp); return CY_ERR;}

    return close_file(fp);
}

CY_STATE_FLAG cypher(const char *inpath, CY_KEY *key, const CY_FUNC cypherfunc, const char *outpath)
{
    CY_String file; uint8_t *buffer;

    if(cypherStart(inpath, &file, &buffer) == CY_ERR) {free_space(&file); return CY_ERR;}

    if(cypherfunc(file, key, &buffer) == CY_ERR) {free(buffer); free_space(&file); return CY_ERR;}

    if(cypherFinish(outpath, file, buffer) == CY_ERR) {free(buffer); free_space(&file); return CY_ERR;}

    free(buffer); return free_space(&file);
}

CY_STATE_FLAG cy_encryption_caesar(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_caesar(*key) == CY_ERR) return CY_ERR;

    uint8_t keya = key->str[0], keyb = key->str[1];

    keya %= 26; keyb %= 26;

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) 
        {
            uint8_t cas = (c >= 'A' && c <= 'Z') ? 'A' : 'a';
            c = (uint8_t) ((keya * (c - cas) + keyb) % 26 + cas);
        }
        (*buffer)[i] = c;
    }
    return CY_OK;
}

CY_STATE_FLAG cy_decryption_caesar(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_caesar(*key) == CY_ERR) return CY_ERR;

    uint64_t keya, keyb = key->str[1];

    if(EEA(key->str[0], 26, &keya) == CY_ERR) return CY_ERR;

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) 
        {
            uint8_t cas = (c >= 'A' && c <= 'Z') ? 'A' : 'a';
            c = (uint8_t) ((keya * (26 + (c - cas) - keyb % 26)) % 26 + cas);
        }
        (*buffer)[i] = c;
    }
    return CY_OK;
}

CY_STATE_FLAG cy_encryption_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_monoalpahbetic(*key) == CY_ERR) return CY_ERR;

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) 
        {
            uint8_t cas = c > 'Z' ? 'a' : 'A';
            c = key->str[c - cas] > 'Z' ? key->str[c - cas] - 'a' + cas : key->str[c - cas] - 'A' + cas;
        }
        (*buffer)[i] = c;
    }
    return CY_OK;
}

CY_STATE_FLAG cy_decryption_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_monoalpahbetic(*key) == CY_ERR) return CY_ERR;

    CY_KEY invkey = {.owner=CY_NOT_OWNED, .size=0, .str=NULL};
    uint8_t abc[] = "abcdefghijklmnopqrstuvwxyz";
    cy_key_linear_inverse_generated('a','z', (CY_KEY){.owner=CY_OWNED, .size=26, .str=abc}, *key, &invkey);
    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) 
        {
            uint8_t cas = c > 'Z' ? 'a' : 'A';
            c = invkey.str[c - cas] > 'Z' ? invkey.str[c - cas] - 'a' + cas : invkey.str[c - cas] - 'A' + cas;
        }
        (*buffer)[i] = c;
    }
    return free_space(&invkey);
}

CY_STATE_FLAG cy_crack_monoalpahbetic(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    CY_KEY invkey = {.owner=CY_NOT_OWNED, .size=0, .str=NULL};
    size_t numtable[26] = {0};
    uint8_t freqchar[] = "etaoinshrdlcumwfgypbvkjxqz";
    uint8_t chartable[] = "abcdefghijklmnopqrstuvwxyz";

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) numtable[(uint8_t) (c - ((c > 'Z') ? 'a' : 'A'))]++;
    }
    for (size_t i = 0; i < 26; i++)
    {
        size_t k = i;

        for (size_t j = i + 1; j < 26; j++) if(numtable[k] < numtable[j]) k = j;

        size_t tmpnum = numtable[i]; numtable[i] = numtable[k]; numtable[k] = tmpnum;

        char tmpchar = chartable[i]; chartable[i] = chartable[k]; chartable[k] = tmpchar;
    }
    cy_key_linear_inverse_generated('a','z', (CY_KEY){.owner=CY_OWNED, .size=26, .str=freqchar}, (CY_KEY){.owner=CY_OWNED, .size=26, .str=chartable}, &invkey);

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) 
        {
            uint8_t cas = c > 'Z' ? 'a' : 'A';
            c = invkey.str[c - cas] > 'Z' ? invkey.str[c - cas] - 'a' + cas : invkey.str[c - cas] - 'A' + cas;
        }
        (*buffer)[i] = c;
    }
    *key = invkey;
    return CY_OK;
}

CY_STATE_FLAG cy_encryption_eascii(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_eascii(*key) == CY_ERR) return CY_ERR;

    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        c = key->str[c];
        (*buffer)[i] = c;
    }

    return CY_OK;
}

CY_STATE_FLAG cy_decryption_eascii(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_eascii(*key) == CY_ERR) return CY_ERR;
    CY_KEY invkey = {.owner=CY_NOT_OWNED, .size=0, .str=NULL};
    uint8_t eascii[256];
    for(uint16_t i = 0; i < 256; i++) eascii[i] = i;

    cy_key_linear_inverse_generated(0, 255, (CY_KEY){.owner=CY_OWNED, .size=256, .str=eascii}, *key, &invkey);
    for (size_t i = 0; i < file.size; i++)
    {
        uint8_t c = file.str[i];
        c = invkey.str[c];
        (*buffer)[i] = c;
    }
    return free_space(&invkey);
}

CY_STATE_FLAG cy_encryption_playfair(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_playfaire(*key) == CY_ERR) return CY_ERR;

    if(realloc_space((void **) buffer, file.size * 2 + 1) == CY_ERR) return CY_ERR;

    size_t size = 0;
    for (size_t i = 0; i < file.size; i++)
    {
        (*buffer)[size] = file.str[i];
        if(i + 1 < file.size && file.str[i] == file.str[i + 1])
        {++size; (*buffer)[size] = key->str[(uint8_t) file.str[i + 1]];}
        size++;
    }

    if (size & 1) {(*buffer)[size] = key->str[ (*buffer)[size - 1] ]; size++;};

    if(realloc_space((void **) buffer, size) == CY_ERR) return CY_ERR;

    for (size_t i = 0; (i + 1) < size; i+=2)
    {
        uint8_t fx = 0, sx = 0, fy = 0, sy = 0;

        find_matrix_pos(key->str, (*buffer)[i], &fx, &fy);

        find_matrix_pos(key->str, (*buffer)[i + 1], &sx, &sy);
        
        if(fx == sx)
        {
            (*buffer)[i] = key->str[fx*16 + (uint8_t)((fy + 1) & 0x0F)];
            (*buffer)[i + 1] = key->str[sx*16 + (uint8_t)((sy + 1) & 0x0F)];
        }
        else if(fy == sy)
        {
            (*buffer)[i] = key->str[((uint8_t)((fx + 1) & 0x0F))*16 + fy];
            (*buffer)[i + 1] = key->str[((uint8_t)((sx + 1) & 0x0F))*16 + sy];
        }
        else
        {
            (*buffer)[i] = key->str[fx*16 + sy];
            (*buffer)[i + 1] = key->str[sx*16 + fy];
        }
    }

    file.size = size;
    return CY_OK;
}

CY_STATE_FLAG cy_decryption_playfair(CY_String file, CY_KEY *key, uint8_t **buffer)
{
    if(cy_check_playfaire(*key) == CY_ERR) return CY_ERR;

    for (size_t i = 0; (i + 1) < file.size; i+=2)
    {
        uint8_t fx = 0, sx = 0, fy = 0, sy = 0;

        find_matrix_pos(key->str, file.str[i], &fx, &fy);

        find_matrix_pos(key->str, file.str[i + 1], &sx, &sy);
        
        if(fx == sx)
        {
            (*buffer)[i] = key->str[fx*16 + (uint8_t)((fy + 15) & 0x0F)];
            (*buffer)[i + 1] = key->str[sx*16 +(uint8_t)((sy + 15) & 0x0F)];
        }
        else if(fy == sy)
        {
            (*buffer)[i] = key->str[((uint8_t)((fx + 15) & 0x0F))*16 + fy];
            (*buffer)[i + 1] = key->str[((uint8_t)((sx + 15) & 0x0F))*16 + sy];
        }
        else
        {
            (*buffer)[i] = key->str[fx*16 + sy];
            (*buffer)[i + 1] = key->str[sx*16 + fy];
        }
    }

    size_t size = 0;
    for (size_t i = 0; (i + 1) < file.size; i++)
    {
        (*buffer)[size] = (*buffer)[i];
        size++;
        if(i + 2 < file.size && (*buffer)[i] == (*buffer)[i + 2] && (*buffer)[i + 1] == key->str[(uint8_t) (*buffer)[i]]) 
        {
            (*buffer)[size] = (*buffer)[i + 2];
            size++; i+=2;
        }
    }
    
    file.size = size;

    if(realloc_space((void **) buffer, size) == CY_ERR) return CY_ERR;

    return CY_OK;
}