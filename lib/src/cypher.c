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

  // Keep original call/semantics (NTSTATUS == 0 on success)
  static inline int cy_random_bytes(void *p, size_t n) {
      return BCryptGenRandom(NULL, (PUCHAR)p, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  }

  #define CY_FSEEK  _fseeki64
  #define CY_FTELL  _ftelli64
  #define __int64_t __int64

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




CY_STATE_FLAG cy_state_manager(const CY_STATE_FLAG e, const char *funcname, const char *msg){
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
    default: fprintf(stderr,"%s(-> ERROR(-> unknown state <-)%s <-)\n", funcname, msg); return CY_ERR;
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




static CY_STATE_FLAG cy_random_mpz(mpz_srcptr n, mpz_ptr out)                                                         
{                                                                                                               
    if (!out || mpz_sgn(n) <= 0) return CY_ERR;                                                                 
                                                                                                                
    size_t bits  = mpz_sizeinbase(n, 2);                                                                        
    size_t bytes = (bits + 7) / 8;                                                                              
    if (bytes == 0) {mpz_set_ui(out, 0); return CY_OK;}                                                      
                                                                                                        
    unsigned char *buf = (unsigned char*) malloc(bytes);                                                        
    if (!buf) return CY_ERR;                                                                                   
                                                                                                            
    for (;;) {                                                                                                
        NTSTATUS st = BCryptGenRandom(NULL, buf, (ULONG)bytes, BCRYPT_USE_SYSTEM_PREFERRED_RNG);                
        if (st != 0) { free(buf); return CY_ERR; }                                                              
                                                                                                                
        mpz_import(out, bytes, 1, 1, 1, 0, buf); /* big-endian bytes → mpz */                                   
        if (mpz_cmp(out, n) < 0) { free(buf); return CY_OK; }                                                   
        /* else: reject and retry */                                                                            
    }                                                                                                           
}

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
DECL_RANDOM_UNIFORM_UNSIGNED(__uint128_t, random_u128)


CY_STATE_FLAG random_u128_full(__uint128_t size, __uint128_t *out) 
{
    if (!out) return CY_ERR;
    NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return st == 0 ? CY_OK : CY_ERR;
}

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




static void gcd(mpz_srcptr a, mpz_srcptr b, mpz_ptr out)
{
    mpz_t tmpa, tmpb; mpz_inits(tmpa, tmpb, NULL);
    mpz_set(tmpa, a); mpz_set(tmpb, b);

    if(!mpz_cmp_ui(tmpa, 0)) {mpz_set(out, tmpb); return;}
    if(!mpz_cmp_ui(tmpb, 0)) {mpz_set(out, tmpa); return;}

    mpz_abs(tmpa, tmpa);
    mpz_abs(tmpb, tmpb);

    mpz_t r; mpz_init(r);
    while (mpz_cmp_ui(tmpb, 0))
    {
        mpz_mod(r, tmpa, tmpb);
        mpz_swap(tmpa, tmpb);
        mpz_swap(tmpb, r);
    }
    mpz_set(out, tmpa);
    mpz_clears(tmpa, tmpb, r, NULL);
}

static CY_STATE_FLAG EEA(mpz_srcptr a, mpz_srcptr n, mpz_ptr out)
{
    gcd(a, n, out);
    if (mpz_cmp_ui(out, 1) != 0)
    {gmp_fprintf(stderr, "gcd(%Zd,%Zd) is different than 1\n", a, n); mpz_set_ui(out, 0); return CY_ERR;}

    mpz_t tmpa, tmpn; mpz_inits(tmpa, tmpn, NULL);
    mpz_set(tmpa, a); mpz_set(tmpn, n);

    // __t represent t-2 and _t represent t-1
    mpz_t __x, _x, mod, x, r; mpz_inits(__x, _x, x, mod, r, NULL);
    mpz_set_ui(__x, 1); mpz_set_ui(_x, 0); mpz_set(mod, tmpn);

    while (mpz_cmp_ui(tmpn, 0))
    {
        mpz_mod(r, tmpa, tmpn);
        mpz_div(out, tmpa, tmpn); mpz_mul(out, _x, out); mpz_sub(x, __x, out);
        mpz_swap(__x, _x); mpz_swap(_x, x);
        mpz_swap(tmpa, tmpn); mpz_swap(tmpn, r);
    }

    mpz_mod(out, __x, mod); mpz_add(out, out, mod); mpz_mod(out, out, mod);
    mpz_clears(tmpa, tmpn, x, _x, __x, mod, r,NULL);

    return CY_OK;
}

static CY_STATE_FLAG __attribute__((unused)) cy_gf2_64_init(const char *coeff, const uint8_t deg, uint64_t *out)
{
    uint16_t exp2 = 1u<<deg; *out = 0;
    for (uint8_t i = 0; i <= deg; i++)
    {
        *out = coeff[i] == '1' ? (*out | (exp2 >> i)) : *out;
        if(coeff[i] != '0' && coeff[i] != '1') return cy_state_manager(CY_ERR_ARG, __func__, ": coeffition unknown");
    }
    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_deg(uint64_t f, uint8_t *deg)
{
    __uint128_t exp2 = 1; exp2 <<= 64; *deg = 0;
    for (uint8_t i = 0; i < 128; i++)
    {
        if(f & exp2 >> i) {*deg =  128 - i; return CY_OK;}
    }
    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_add(uint64_t f, uint64_t g, uint64_t *out)
{
    *out = f ^ g;
    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_sub(uint64_t f, uint64_t g, uint64_t *out)
{
    *out = f ^ g;
    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_mul(uint64_t f, uint64_t g, __uint128_t *out)
{
    __uint128_t r = 0;
    while (g){
        if (g & 1) r ^= f;   // add current f if LSB of g is 1
        g >>= 1;             // next bit of g
        f <<= 1;             // corresponds to shifting by +1
    }
    *out = r;

    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_div(uint64_t f, uint64_t g, uint64_t *q, uint64_t *r)
{
    *q = 0; *r = f; uint64_t h = 0; uint8_t dr = 0, dg = 0, dh = 0;
    cy_gf2_64_deg(g, &dg);
    while(1)
    {
        cy_gf2_64_deg(*r, &dr);
        if(dr < dg) break;
        dh = dr - dg;
        h = g << dh;
        cy_gf2_64_sub(*r, h, r);
        *q |= 1u<<dh;  
    }
    
    return CY_OK;
}

static CY_STATE_FLAG cy_gf2_64_mod(uint64_t f, uint64_t g, uint64_t *mod)
{
    uint64_t h = 0;
    cy_gf2_64_div(f, g, &h, mod);
    return CY_OK;
}

void cy_gf2_64_gcd(uint64_t a, uint64_t b, uint64_t *out)
{
    if(!a) {*out = b; return;}
    if(!b) {*out = a; return;}

    while (b)
    {
        uint64_t r;
        cy_gf2_64_mod(a, b, &r);
        a = b;
        b = r;
    }
    *out = a;
}

static CY_STATE_FLAG __attribute__((unused)) cy_gf2_64_eea(uint64_t a, uint64_t n, uint64_t *out)
{
    uint64_t _gcd = 0; cy_gf2_64_gcd(a, n, &_gcd);
    if(_gcd != 1) 
    {fprintf(stderr, "gcd(%"PRIu64",%"PRIu64") is different than 1\n", a, n); return CY_ERR;}

    // __t represent t-2 and _t represent t-1
    uint64_t __x = 1, _x = 0, mod = n;
    while (n)
    {
        __uint128_t x = 0; 
        uint64_t r = 0, q = 0, rem = 0;
        cy_gf2_64_mod(a, n, &r);
        cy_gf2_64_div(a, n, &q, &rem);
        cy_gf2_64_mul(q, _x, &x);
        x = __x ^ x;
        // __uint128_t x = __x - (a / n)*_x;
        __x = _x; _x = (uint64_t) x;
        a = n;
        n = r;
    }

    cy_gf2_64_mod(__x, mod, &__x);
    cy_gf2_64_add(__x, mod, &__x);
    cy_gf2_64_mod(__x, mod, out);

    return CY_OK;
}

static CY_STATE_FLAG __attribute__((unused)) cy_gf2_64_printf(const uint64_t f)
{
    uint8_t deg;
    cy_gf2_64_deg(f, &deg);
    __uint128_t mask = 1;
    mask <<= deg;     // 64-bit mask
    int printed = 0;                 // have we printed any term yet?

    for (uint8_t i = 0; i <= deg; i++) {
        uint8_t e = (uint8_t)(deg - i);
        if ((f & (mask >> i)) && e != 0) {   // skip e==0 (no X^(0))
            if (printed) printf(" + ");
            printf("X^(%" PRIu8 ")", e);
            printed = 1;
        }
    }

    if (f & 1ULL) {                  // print constant only if present
        if (printed) printf(" + ");
        printf("1");
        printed = 1;
    }

    if (!printed) printf("0");       // nothing set -> print 0
    printf("\n");
    return CY_OK;
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




static const uint8_t CY_AES_SBOX[16][16] = 
{
    {0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76},
    {0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0},
    {0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15},
    {0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75},
    {0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84},
    {0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF},
    {0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8},
    {0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2},
    {0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73},
    {0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB},
    {0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79},
    {0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08},
    {0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A},
    {0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E},
    {0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF},
    {0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16}
};

static const uint8_t CY_AES_INVSBOX[16][16] = 
{
    {0x52,0x09,0x6A,0xD5,0x30,0x36,0xA5,0x38,0xBF,0x40,0xA3,0x9E,0x81,0xF3,0xD7,0xFB},
    {0x7C,0xE3,0x39,0x82,0x9B,0x2F,0xFF,0x87,0x34,0x8E,0x43,0x44,0xC4,0xDE,0xE9,0xCB},
    {0x54,0x7B,0x94,0x32,0xA6,0xC2,0x23,0x3D,0xEE,0x4C,0x95,0x0B,0x42,0xFA,0xC3,0x4E},
    {0x08,0x2E,0xA1,0x66,0x28,0xD9,0x24,0xB2,0x76,0x5B,0xA2,0x49,0x6D,0x8B,0xD1,0x25},
    {0x72,0xF8,0xF6,0x64,0x86,0x68,0x98,0x16,0xD4,0xA4,0x5C,0xCC,0x5D,0x65,0xB6,0x92},
    {0x6C,0x70,0x48,0x50,0xFD,0xED,0xB9,0xDA,0x5E,0x15,0x46,0x57,0xA7,0x8D,0x9D,0x84},
    {0x90,0xD8,0xAB,0x00,0x8C,0xBC,0xD3,0x0A,0xF7,0xE4,0x58,0x05,0xB8,0xB3,0x45,0x06},
    {0xD0,0x2C,0x1E,0x8F,0xCA,0x3F,0x0F,0x02,0xC1,0xAF,0xBD,0x03,0x01,0x13,0x8A,0x6B},
    {0x3A,0x91,0x11,0x41,0x4F,0x67,0xDC,0xEA,0x97,0xF2,0xCF,0xCE,0xF0,0xB4,0xE6,0x73},
    {0x96,0xAC,0x74,0x22,0xE7,0xAD,0x35,0x85,0xE2,0xF9,0x37,0xE8,0x1C,0x75,0xDF,0x6E},
    {0x47,0xF1,0x1A,0x71,0x1D,0x29,0xC5,0x89,0x6F,0xB7,0x62,0x0E,0xAA,0x18,0xBE,0x1B},
    {0xFC,0x56,0x3E,0x4B,0xC6,0xD2,0x79,0x20,0x9A,0xDB,0xC0,0xFE,0x78,0xCD,0x5A,0xF4},
    {0x1F,0xDD,0xA8,0x33,0x88,0x07,0xC7,0x31,0xB1,0x12,0x10,0x59,0x27,0x80,0xEC,0x5F},
    {0x60,0x51,0x7F,0xA9,0x19,0xB5,0x4A,0x0D,0x2D,0xE5,0x7A,0x9F,0x93,0xC9,0x9C,0xEF},
    {0xA0,0xE0,0x3B,0x4D,0xAE,0x2A,0xF5,0xB0,0xC8,0xEB,0xBB,0x3C,0x83,0x53,0x99,0x61},
    {0x17,0x2B,0x04,0x7E,0xBA,0x77,0xD6,0x26,0xE1,0x69,0x14,0x63,0x55,0x21,0x0C,0x7D}
};

static const uint8_t CY_AES_MIXCOL_MAT[4][4] = 
{
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
};

static const uint8_t CY_AES_INVMIXCOL_MAT[4][4] = 
{
    {0x0E, 0x0B, 0x0D, 0x09},
    {0x09, 0x0E, 0x0B, 0x0D},
    {0x0D, 0x09, 0x0E, 0x0B},
    {0x0B, 0x0D, 0x09, 0x0E}
};

static const uint8_t CY_RC[10] = 
{
    0x01, 0x02, 0x04, 0x08, 0x10,
    0x20, 0x40, 0x80, 0x1B, 0x36
};

static CY_STATE_FLAG cy_rsa_prime_prob_gen(const mp_bitcnt_t bitsize, mpz_ptr p)
{
    mpz_t n; mpz_init(n);
    mpz_setbit(n, bitsize);
    do
    {cy_random_mpz(n, p);}
    while(mpz_probab_prime_p(p, 100) == 0 );

    mpz_clear(n);
    return CY_OK;
}

void cy_aes_from_128_to_4by4(const __uint128_t num, uint8_t tab[4][4])
{
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            tab[j][i] = (uint8_t) (num >> (i*32+j*8)) & 0xFF;
        }
    }
}

void cy_aes_substitute_bytes(const uint8_t sbox[16][16], uint8_t state[4][4])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            uint8_t row = state[i][j] >> 4, column = state[i][j] & 0x0F;
            state[i][j] = sbox[row][column];
        } 
    }
}

void cy_aes_shift_rows(uint8_t state[4][4])
{

    uint8_t row[4] = {0};
    row[0] = state[1][1]; row[1] = state[1][2]; row[2] = state[1][3]; row[3] = state[1][0];
    state[1][0] = row[0]; state[1][1] = row[1]; state[1][2] = row[2]; state[1][3] = row[3];

    row[0] = state[2][2]; row[1] = state[2][3]; row[2] = state[2][0]; row[3] = state[2][1];
    state[2][0] = row[0]; state[2][1] = row[1]; state[2][2] = row[2]; state[2][3] = row[3];

    row[0] = state[3][3]; row[1] = state[3][0]; row[2] = state[3][1]; row[3] = state[3][2];
    state[3][0] = row[0]; state[3][1] = row[1]; state[3][2] = row[2]; state[3][3] = row[3];
}

void cy_aes_invshift_rows(uint8_t state[4][4])
{
    uint8_t row[4] = {0};
    row[0] = state[1][3]; row[1] = state[1][0]; row[2] = state[1][1]; row[3] = state[1][2];
    state[1][0] = row[0]; state[1][1] = row[1]; state[1][2] = row[2]; state[1][3] = row[3];

    row[0] = state[2][2]; row[1] = state[2][3]; row[2] = state[2][0]; row[3] = state[2][1];
    state[2][0] = row[0]; state[2][1] = row[1]; state[2][2] = row[2]; state[2][3] = row[3];

    row[0] = state[3][1]; row[1] = state[3][2]; row[2] = state[3][3]; row[3] = state[3][0];
    state[3][0] = row[0]; state[3][1] = row[1]; state[3][2] = row[2]; state[3][3] = row[3];
}

void cy_aes_mix_columns(const uint8_t mix_c_matrix[4][4], uint8_t state[4][4])
{
    uint8_t temp[4][4]= {{0}};
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            __uint128_t C = 0; uint64_t res = 0;
            for (size_t k = 0; k < 4; k++)
            {
                cy_gf2_64_mul(mix_c_matrix[i][k], state[k][j], &C);
                cy_gf2_64_mod(C, 0x11B, &res);
                temp[i][j] ^= (uint8_t) res;
            }
        }
    }
    for (uint8_t i = 0; i < 4; i++)
        for (uint8_t j = 0; j < 4; j++)
            state[i][j] = temp[i][j];
}

void cy_aes_add_round_key(const uint32_t key[4], uint8_t state[4][4])
{
    uint8_t temp[4][4];
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            temp[j][i] = state[j][i] ^ ((key[i]>>(8*j)) & 0xFF);
        }
    }
    for (uint8_t i = 0; i < 4; i++)
        for (uint8_t j = 0; j < 4; j++)
            state[i][j] = temp[i][j];
}

void cy_aes_g_function(const uint8_t j, uint32_t *w)
{
    uint8_t b[4];
    for (size_t i = 0; i < 4; i++) 
    {
        b[i] = (uint8_t)((*w>>(((i + 1)%4)*8)) & 0xFF);
        uint8_t row = b[i] >> 4, column = b[i] & 0x0F;
        b[i] = CY_AES_SBOX[row][column];
    }

    b[0] ^= CY_RC[j];
    *w = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

void cy_aes_key_expansion(__uint128_t key, uint32_t w[44])
{
    uint8_t tabkey[4][4];
    cy_aes_from_128_to_4by4(key, tabkey);
    w[0] = (uint32_t) tabkey[0][0] | ((uint32_t) tabkey[1][0] << 8) | ((uint32_t) tabkey[2][0] << 16) | ((uint32_t) tabkey[3][0] << 24);
    w[1] = (uint32_t) tabkey[0][1] | ((uint32_t) tabkey[1][1] << 8) | ((uint32_t) tabkey[2][1] << 16) | ((uint32_t) tabkey[3][1] << 24);
    w[2] = (uint32_t) tabkey[0][2] | ((uint32_t) tabkey[1][2] << 8) | ((uint32_t) tabkey[2][2] << 16) | ((uint32_t) tabkey[3][2] << 24);
    w[3] = (uint32_t) tabkey[0][3] | ((uint32_t) tabkey[1][3] << 8) | ((uint32_t) tabkey[2][3] << 16) | ((uint32_t) tabkey[3][3] << 24);
    for (uint8_t i = 4; i < 44; i++)
    {
        uint32_t temp = w[i - 1];
        if(i%4 == 0) cy_aes_g_function((uint8_t) (i/4 - 1), &temp);
        w[i] = w[i - 4] ^ temp;
    }
}

void cy_aes_from_4by4_to_128(const uint8_t tab[4][4], __uint128_t *num)
{
    *num = 0;
    for (uint8_t c = 0; c < 4; c++) 
    {
        for (uint8_t r = 0; r < 4; r++) 
        {
            *num |= ((__uint128_t) tab[r][c]) << 8u * (4u*c + r);
        }
    }
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
    if(!fp) return cy_state_manager(CY_ERR_ARG, __func__, ": fp is NULL");
    if(!mode || !*mode) return cy_state_manager(CY_ERR_ARG, __func__, ": mode is NULL/empty");
    if(!path || !*path) return cy_state_manager(CY_ERR_ARG, __func__, ": path is NULL/empty");

    *fp = NULL; *fp = fopen(path, mode);
    if(!*fp) {perror("fopen"); return cy_state_manager(CY_ERR_OPEN, __func__, "");}

    return CY_OK;
}

static CY_STATE_FLAG close_file(FILE *fp)
{
    if (!fp) return cy_state_manager(CY_ERR_ARG, __func__, ": fp is NULL");

    if (fclose(fp) != 0) 
    {perror("fclose"); return cy_state_manager(CY_ERR_CLOSE, __func__, ": fclose faild to close fp");}

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









/******************************************************** 
 * 
 * 
 * 
 * 
 *                     Key Functions 
 *
 * 
 * 
 * 
 *********************************************************/




CY_STATE_FLAG cy_rsa_key_gen(const mp_bitcnt_t bitsize, mpz_t **pubkey, mpz_t **prvkey)
{
    mpz_t n, p, q, phi_n, e, d;
    *pubkey = malloc(2 * sizeof((*pubkey)[0]));
    *prvkey = malloc(2 * sizeof((*prvkey)[0]));
    mpz_inits((*pubkey)[0], (*pubkey)[1], (*prvkey)[0], (*prvkey)[1], n, p, q, phi_n, e, d, NULL);
    cy_rsa_prime_prob_gen(bitsize, p);
    cy_rsa_prime_prob_gen(bitsize, q);
    mpz_mul(n, p, q);
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_mul(phi_n, p, q);
    cy_rsa_prime_prob_gen(bitsize / 10, e);
    if(EEA(e, phi_n, d) == CY_ERR) return CY_ERR;
    mpz_set((*pubkey)[0], e); mpz_set((*pubkey)[1], n);
    mpz_set((*prvkey)[0], d); mpz_set((*prvkey)[1], n);
    mpz_clears(e, d, n, p, q, phi_n, NULL);
    return CY_OK;
}

CY_STATE_FLAG cy_rsa_key_imp(const char *path, mpz_t **key)
{
    FILE *fp;
    *key = malloc(2 * sizeof((*key)[0]));

    mpz_inits((*key)[0], (*key)[1], NULL);
    if(open_file(&fp, "rb", path) == CY_ERR) return CY_ERR;
    gmp_fscanf(fp, "%Zd\n%Zd", (*key)[0], (*key)[1]);
    if(close_file(fp) == CY_ERR) return CY_ERR;
    return CY_OK;
}

CY_STATE_FLAG cy_rsa_key_exp(const char *path, const mpz_t *key)
{
    FILE *fp;
    if(open_file(&fp, "wb", path) == CY_ERR) return CY_ERR;
    gmp_fprintf(fp, "%Zd\n%Zd", key[0], key[1]);
    if(close_file(fp) == CY_ERR) return CY_ERR;
    return CY_OK;
}

CY_STATE_FLAG cy_aes_key_gen(__uint128_t *key)
{
    __uint128_t size = 0;
    random_u128(17, &size);
    if(random_u128_full(size, key) == CY_ERR) return cy_state_manager(CY_ERR_KEY, __func__, ": aes key generation failed");
    *key *= size;
    return CY_OK;
}

CY_STATE_FLAG cy_aes_key_imp(const char *path, __uint128_t *key)
{
    FILE *fp; uint8_t b[16]={0}; *key = 0;
    if(open_file(&fp, "rb", path) == CY_ERR) return CY_ERR;
    size_t size = fread(b, 1, 16, fp);
    if(size == 0) return cy_state_manager(CY_ERR_IO, __func__, ": didnt read the key");
    for (int i=0; i <16; i++) *key |= ((__uint128_t)(b[i]))<<(i * 8);
    if(close_file(fp) == CY_ERR) return CY_ERR;
    return CY_OK;
}

CY_STATE_FLAG cy_aes_key_exp(const char *path, __uint128_t key)
{
    FILE *fp; uint8_t b[16]={0};
    if(open_file(&fp, "wb", path) == CY_ERR) return CY_ERR;
    for (int i=0; i <16; i++) b[i] = (uint8_t)((key >> (i * 8)) & 0xFF);
    fwrite(b, 1, 16, fp);
    if(close_file(fp) == CY_ERR) return CY_ERR;
    return CY_OK;
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




void cy_rsa_encryption(const uint8_t c, const mpz_t *key, mpz_ptr cy_msg)
{
    mpz_set_ui(cy_msg, c);
    mpz_powm(cy_msg, cy_msg, key[0], key[1]);
}

void cy_rsa_decryption(const mpz_srcptr cy_msg, const mpz_t *key, uint8_t *c)
{
    mpz_t out; mpz_init(out);
    mpz_powm(out, cy_msg, key[0], key[1]);
    *c = (uint8_t) mpz_get_ui(out);
    mpz_clear(out);
}

void cy_aes_encryption(__uint128_t msg, __uint128_t key, __uint128_t *cy_msg)
{
    uint32_t expandkey[44]; uint8_t state[4][4]; *cy_msg = 0;
    cy_aes_from_128_to_4by4(msg, state);
    cy_aes_key_expansion(key, expandkey);
    cy_aes_add_round_key(expandkey, state);

    for (uint8_t i = 1; i < 10; i++)
    {
        cy_aes_substitute_bytes(CY_AES_SBOX, state);
        cy_aes_shift_rows(state);
        cy_aes_mix_columns(CY_AES_MIXCOL_MAT, state);
        cy_aes_add_round_key(&expandkey[4 * i], state);
    }
    cy_aes_substitute_bytes(CY_AES_SBOX, state);
    cy_aes_shift_rows(state);
    cy_aes_add_round_key(&expandkey[40], state);
    cy_aes_from_4by4_to_128(state, cy_msg);
}

void cy_aes_decryption(__uint128_t cy_msg, __uint128_t key, __uint128_t *msg)
{
    uint32_t expandkey[44]; uint8_t state[4][4]; *msg = 0;
    cy_aes_from_128_to_4by4(cy_msg, state);
    cy_aes_key_expansion(key, expandkey);
    cy_aes_add_round_key(expandkey + 40, state);

    for (uint8_t i = 1; i < 10; i++)
    {
        cy_aes_invshift_rows(state);
        cy_aes_substitute_bytes(CY_AES_INVSBOX, state);
        cy_aes_add_round_key(expandkey + (40 - 4 * i), state);
        cy_aes_mix_columns(CY_AES_INVMIXCOL_MAT, state);
    }
    cy_aes_invshift_rows(state);
    cy_aes_substitute_bytes(CY_AES_INVSBOX, state);
    cy_aes_add_round_key(expandkey, state);
    cy_aes_from_4by4_to_128(state, msg);
}




/******************************************************** 
 * 
 * 
 * 
 * 
 *                 Buffered Cypher Functions 
 *
 * 
 * 
 * 
 *********************************************************/




void cy_buff_padding_add(const size_t size, uint8_t *pad, uint8_t buffer[])
{
    *pad = 16 - size % 16;
    for (size_t i = 0; i < *pad; i++) buffer[size + i] = *pad;
}

void cy_buff_size_exp(const size_t size, uint8_t buff[])
{
    for (size_t i = 0; i < sizeof(size_t); i++)
        buff[sizeof(size_t) - (i + 1)] = (uint8_t)((size >> (i * 8)) & 0xFF);  
}

void cy_buff_size_imp(const uint8_t buff[], size_t *size)
{
    *size = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) 
        *size |= ((size_t) (buff[sizeof(size_t) - (i + 1)])) << (i * 8);
}

void cy_buff_msg_128_exp(const __uint128_t msg, uint8_t buff[])
{
    for (__uint128_t i = 0; i < sizeof(__uint128_t); i++)
        buff[sizeof(__uint128_t) - (i + 1)] = (uint8_t)((msg >> (i * 8)) & 0xFF);  
}

void cy_buff_msg_128_imp(const uint8_t buff[], __uint128_t *size)
{
    *size = 0;
    for (__uint128_t i = 0; i < sizeof(__uint128_t); i++) 
        *size |= ((__uint128_t) (buff[sizeof(__uint128_t) - (i + 1)])) << (i * 8); 
}

void cy_buff_header_exp(const size_t size, const CY_CYPHER_TYPE cy_type, const uint8_t pad, uint8_t buff[])
{
    cy_buff_size_exp(size, buff);
    buff[8] = (uint8_t) cy_type;
    buff[9] = pad;
}

void cy_buff_header_imp(const uint8_t buff[], size_t *size, CY_CYPHER_TYPE *cy_type, uint8_t *pad)
{
    cy_buff_size_imp(buff, size);
    *cy_type = (CY_CYPHER_TYPE) buff[8];
    *pad = buff[9];
}

void cy_buff_rsa_key_exp(mpz_t *key, uint8_t buff[])
{
    size_t sizekey0 = 0, sizekey1 = 0;
    mpz_export(buff + 8, &sizekey0, 1, sizeof(buff[0]), 0, 0, key[0]);
    cy_buff_size_exp(sizekey0, buff);
    mpz_export(buff + sizekey0 + 16, &sizekey1, 1, sizeof(buff[0]), 0, 0, key[1]);
    cy_buff_size_exp(sizekey1, buff + 8 + sizekey0);
}

void cy_buff_rsa_key_imp(const uint8_t buff[], mpz_t **key)
{
    size_t sizekey0 = 0, sizekey1 = 0;
    cy_buff_size_imp(buff, &sizekey0);
    mpz_import((*key)[0], sizekey0, 1, sizeof(buff[0]), 0, 0, buff + 8);
    cy_buff_size_imp(buff + 8 + sizekey0, &sizekey1);
    mpz_import((*key)[1], sizekey1, 1, sizeof(buff[0]), 0, 0, buff + sizekey0 + 16);
}

void cy_buff_aes_key_exp(const __uint128_t key, uint8_t buff[])
{
    cy_buff_msg_128_exp(key, buff);
}

void cy_buff_aes_key_imp(const uint8_t buff[], __uint128_t *key)
{
    cy_buff_msg_128_imp(buff, key);
}

void cy_buff_rsa_encryption(const size_t size, const mpz_t *key, const size_t buffn, uint8_t buff[])
{
    size_t msgsize = 0, offset = 0;
    mpz_t cy_msg; mpz_init(cy_msg); uint8_t bufftmp[buffn];
    for (size_t i = 0; i < buffn; i++) bufftmp[i] = buff[i];

    for (size_t i = 0; i < size; i++)
    {
        cy_rsa_encryption(bufftmp[i], key, cy_msg);
        mpz_export(buff + 8 + offset, &msgsize, 1, 1, 1, 0, cy_msg);
        cy_buff_size_exp(msgsize, buff + offset);
        offset += 8 + msgsize;
    }
    mpz_clear(cy_msg);
}

void cy_buff_rsa_decryption(const size_t size, const mpz_t *key, const size_t buffn, uint8_t buff[])
{
    size_t msgsize = 0, offset = 0;
    mpz_t cy_msg; mpz_init(cy_msg); uint8_t bufftmp[buffn];
    for (size_t i = 0; i < buffn; i++) bufftmp[i] = buff[i];

    for (size_t i = 0; i < size; i++)
    {
        cy_buff_size_imp(bufftmp + offset , &msgsize);
        mpz_import(cy_msg, msgsize, 1, 1, 1, 0, bufftmp + 8 + offset);
        cy_rsa_decryption(cy_msg, key, buff + i);
        offset += 8 + msgsize;
    }
    mpz_clear(cy_msg);
}

void cy_buff_aes_encryption(const size_t size, const __uint128_t key, uint8_t buff[])
{
    __uint128_t msg, cy_msg;
    size_t sizen = size / 16;
    for (size_t i = 0; i < sizen; i++)
    {
        cy_buff_msg_128_imp(buff + i * 16, &msg);
        cy_aes_encryption(msg, key, &cy_msg);
        cy_buff_msg_128_exp(cy_msg, buff + i * 16);
    }
}

void cy_buff_aes_decryption(const size_t size, const __uint128_t key, uint8_t buff[])
{
    __uint128_t msg, cy_msg;
    size_t sizen = size / 16;
    for (size_t i = 0; i < sizen; i++)
    {
        cy_buff_msg_128_imp(buff + i * 16, &cy_msg);
        cy_aes_decryption(cy_msg, key, &msg);
        cy_buff_msg_128_exp(msg, buff + i * 16);
    }
}

// typedef struct DoubleNode
// {
//     __uint128_t data;
//     struct DoubleNode *next;
//     struct DoubleNode *prev;
// }DoubleNode;

// typedef struct DoubleLinkedList
// {
//     size_t len;
//     DoubleNode *head;
//     DoubleNode *tail;
// }DoubleLinkedList, Stack, Queue;

// DoubleLinkedList new_DoubleLinkedList()
// {
//     return (DoubleLinkedList) {.len=0, .head=NULL, .tail=NULL};
// }

// Stack new_Stack()
// {
//     return new_DoubleLinkedList();
// }

// Queue new_Queue()
// {
//     return new_DoubleLinkedList();
// }

// int add_DoubleNode_end(DoubleLinkedList *list, const __uint128_t data)
// {
//     if(!list)
//     {fprintf(stderr, "Cant add node to null list"); return -1;}

//     DoubleNode *newdoublenode = malloc(sizeof(DoubleNode));
//     if(!newdoublenode)
//     {perror("add_DoubleNode(-> malloc <-)"); return -1;}
//     newdoublenode->data = data;
//     newdoublenode->next = NULL;
//     newdoublenode->prev = NULL;
//     list->len += 1;

//     if(!list->head)
//     {
//         list->head = list->tail = newdoublenode;
//     }
//     else
//     {
//         list->tail->next = newdoublenode;
//         newdoublenode->prev = list->tail;
//         list->tail = newdoublenode;
//     }
//     return 0;
// }

// void push(Stack *stack, const __uint128_t data)
// {
//     add_DoubleNode_end(stack, data);
// }

// int add_DoubleNode_start(DoubleLinkedList *list, const __uint128_t data)
// {
//     if(!list)
//     {fprintf(stderr, "Cant add node to null list"); return -1;}

//     DoubleNode *newdoublenode = malloc(sizeof(DoubleNode));
//     if(!newdoublenode)
//     {perror("add_DoubleNode(-> malloc <-)"); return -1;}
//     newdoublenode->data = data;
//     newdoublenode->next = NULL;
//     newdoublenode->prev = NULL;
//     list->len += 1;

//     if(!list->head)
//     {
//         list->head = list->tail = newdoublenode;
//     }
//     else
//     {
//         list->head->prev = newdoublenode;
//         newdoublenode->next = list->head;
//         list->head = newdoublenode;
//     }
//     return 0;
// }

// void enqueue(Queue *queue, const __uint128_t data)
// {
//     add_DoubleNode_end(queue, data);
// }

// __uint128_t remove_DoubleNode_end(DoubleLinkedList *list)
// {
//     if(!list || list->len == 0)
//     {fprintf(stderr, "Cant remove node from null/empty list"); return 0;}

//     DoubleNode *to_remove = list->tail;
//     __uint128_t data = to_remove->data;
//     list->len -= 1;

//     if(list->head == list->tail)
//     {
//         list->head = list->tail = NULL;
//     }
//     else
//     {
//         list->tail = to_remove->prev;
//         list->tail->next = NULL;
//     }

//     free(to_remove);
//     return data;
// }

// __uint128_t pop(Stack *stack)
// {
//     return remove_DoubleNode_end(stack);
// }

// __uint128_t remove_DoubleNode_start(DoubleLinkedList *list)
// {
//     if(!list || list->len == 0)
//     {fprintf(stderr, "Cant remove node from null/empty list"); return 0;}

//     DoubleNode *to_remove = list->head;
//     __uint128_t data = to_remove->data;
//     list->len -= 1;

//     if(list->head == list->tail)
//     {
//         list->head = list->tail = NULL;
//     }
//     else
//     {
//         list->head = to_remove->next;
//         list->head->prev = NULL;
//     }

//     free(to_remove);
//     return data;
// }

// __uint128_t dequeue(Queue *queue)
// {
//     return remove_DoubleNode_start(queue);
// }

// void free_DoubleLinkedList(DoubleLinkedList *list)
// {
//     if(!list)
//     {fprintf(stderr, "Cant free null list"); return;}

//     DoubleNode *current = list->head;
//     DoubleNode *next;

//     while(current)
//     {
//         next = current->next;
//         free(current);
//         current = next;
//     }

//     list->head = list->tail = NULL;
//     list->len = 0;
// }

// void free_Stack(Stack *stack)
// {
//     free_DoubleLinkedList(stack);
// }

// void free_Queue(Queue *queue)
// {
//     free_DoubleLinkedList(queue);
// }

// void print_DoubleLinkedList(const DoubleLinkedList *list, const char *name)
// {
//     if(!list)
//     {fprintf(stderr, "Cant print null list"); return;}

//     DoubleNode *current = list->head;
//     printf("%s: NULL <-> ", name);
//     while(current)
//     {
//         printf("%lu <-> ", (unsigned long)current->data);
//         current = current->next;
//     }
//     printf("NULL");
//     printf("\n");
// }

// void print_DoubleLinkedList_reverse(const DoubleLinkedList *list, const char *name)
// {
//     if(!list)
//     {fprintf(stderr, "Cant print null list"); return;}

//     DoubleNode *current = list->tail;
//     printf("%s: NULL <-> ", name);
//     while(current)
//     {
//         printf("%lu <-> ", (unsigned long)current->data);
//         current = current->prev;
//     }
//     printf("NULL");
//     printf("\n");
// }

// void print_Stack(const Stack *stack)
// {
//     print_DoubleLinkedList(stack, "Stack");
// }

// void print_Queue(const Queue *queue)
// {
//     print_DoubleLinkedList_reverse(queue, "Queue");
// }