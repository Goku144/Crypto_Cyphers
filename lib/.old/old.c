// static CY_STATE_FLAG MRA(mpz_srcptr p, CY_PRIMALITY_FLAG *out)
// {
//     *out = CY_INCONCLUSIVE;

//     if(mpz_cmp_ui(p, 3) < 0) 
//     {gmp_fprintf(stderr, "%Zd < 3, decomposition condition unsatisfied.\n", p); return CY_ERR;}

//     mpz_t comp, a, q, a_q; mpz_inits(comp, a, q, a_q, NULL); 
//     mpz_sub_ui(comp, p, 1);
//     if(random_mpz(comp, a) == CY_ERR) return CY_ERR;

//     if(mpz_cmp_ui(a, 2) < 0) mpz_set_ui(a, 2); // 1 < a < n - 1

//     mpz_sub_ui(comp, p, 1);
//     mp_bitcnt_t k = mpz_scan1(comp, 0);
//     mpz_fdiv_q_2exp(q, comp, k);
//     mpz_powm(a_q, a, q, p);

//     if(mpz_cmp_ui(a_q, 1) == 0) return CY_OK;

//     for (size_t i = 0; i < k; i++)
//     {
//         mpz_mul(a_q, a_q, a_q);
//         mpz_mod(a_q, a_q, p);
//         if(mpz_cmp_ui(a_q, 1) == 0) return CY_OK;
//     }
    
//     mpz_clears(comp, a, q, a_q, NULL);
//     *out = CY_COMPOSITE;
//     return CY_OK;
// }

// static CY_STATE_FLAG EMRA(mpz_srcptr p, const uint64_t prob, CY_PRIMALITY_FLAG *out)
// {
//     if (MRA(p,out) == CY_ERR) return CY_ERR;

//     for (__uint128_t i = 0; i < prob; i++) 
//     {
//         MRA(p,out);
//         if(*out == CY_COMPOSITE) return CY_OK;
//     }

//     return CY_OK;  
// }



// static CY_STATE_FLAG malloc_space(void **memp, const size_t size)
// {
//     if (!memp) return cy_state_manager(CY_ERR_ARG, __func__, ": memp is NULL");
//     if (!size) return cy_state_manager(CY_ERR_ARG, __func__, ": size is 0");

//     *memp = NULL;
//     *memp = malloc(size);

//     if (!*memp) 
//     {perror("malloc"); return cy_state_manager(CY_ERR_OOM, __func__, ": malloc faild to allocate space");}

//     return CY_OK;
// }

// static CY_STATE_FLAG realloc_space(void **memp, const size_t size)
// {
//         if (!memp) return cy_state_manager(CY_ERR_ARG, __func__, ": memp is NULL");
//     if (!size) return cy_state_manager(CY_ERR_ARG, __func__, ": size is 0");

//     *memp = realloc(*memp, size);

//     if (!*memp) 
//     {perror("realloc"); return cy_state_manager(CY_ERR_OOM, __func__, ": realloc faild to reallocate space");}

//     return CY_OK;
// }

// static CY_STATE_FLAG free_space(CY_String *file)
// {
//     if (!file || !file->str) return cy_state_manager(CY_ERR_ARG, __func__, ": file/file->str is NULL");

//     if (file->owner == CY_OWNED) free(file->str);

//     file->str = NULL; file->owner = CY_NOT_OWNED; file->size = 0;

//     return CY_OK;
// }

// static CY_STATE_FLAG read_file(FILE *fp, void *dst, const size_t size, size_t *offset_out)
// {
//     if (!fp)  return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
//     if (!dst) return cy_err_manager(CY_ERR_ARG, __func__, ": dst is NULL");
//     if (!size) return CY_OK;

//     uint8_t *p = (uint8_t *)dst;
//     size_t offset = 0;

//     while (offset < size) {
//         size_t n = fread(p + offset, 1, size - offset, fp);
//         if (n == 0) {
//             if (ferror(fp)) {
//                 perror("fread");
//                 return cy_err_manager(CY_ERR_IO, __func__, ": fread failed");
//             }
            
//             if (feof(fp)) {
//                 clearerr(fp);           // allow re-use of fp
//                 *offset_out = offset;
//                 return CY_INFO_EOF;        // EOF reached (not an error)
//             }

//             return cy_err_manager(CY_ERR_EOF, __func__, ": unexpected EOF");
//         }
//         offset += n;
//     }

//     *offset_out = offset;
//     return CY_OK;
// }

// static CY_STATE_FLAG read_fchar(FILE *fp, int *c)
// {
//     if(!fp) return cy_err_manager(CY_ERR_ARG, __func__, ": fp is NULL");
//     if ((c = fgetc(fp)) == EOF)
//         return CY_INFO_EOF;
//     else if(ferror(fp))
//         return cy_err_manager(CY_ERR_IO, __func__, ": fread failed");
//     return CY_OK;
// }

// static CY_STATE_FLAG write_file(FILE *fp, const void *src, const size_t size, size_t *offset_out)
// {
//     if (!fp) return cy_state_manager(CY_ERR_ARG, __func__, ": fp is NULL");
//     if (!src) return cy_state_manager(CY_ERR_ARG, __func__, ": src is NULL");
//     if (!size) { *offset_out = 0; return CY_OK; }

//     const uint8_t *p = (const uint8_t *)src;
//     size_t offset = 0;

//     while (offset < size) 
//     {
//         size_t n = fwrite(p + offset, 1, size - offset, fp);
//         if (n == 0) 
//         {
//             if (ferror(fp)) {
//                 perror("fwrite");
//                 *offset_out = offset;
//                 return cy_state_manager(CY_ERR_IO, __func__, ": fwrite faild to write the full file");
//             }
//             *offset_out = offset;
//             return cy_state_manager(CY_ERR_INTERNAL, __func__, ": fwrite made no progress");
//         }
//         offset += n;
//     }

//     *offset_out = offset;
//     return CY_OK;
// }

// FILE *fpin, *fpout; char c; mpz_t msg; mpz_init(msg);
//     if(open_file(&fpin, "rb", inpath) == CY_ERR) return CY_ERR;
//     if(open_file(&fpout, "wb", outpath) == CY_ERR) return CY_ERR;

//     while(gmp_fscanf(fpin, "%c", &c) != EOF)
//     {
//         mpz_set_ui(msg, (uint8_t) c);
//         mpz_powm(msg, msg, key.str[0], key.str[1]);
//         gmp_fprintf(fpout, "%Zd\n", msg);
//     }

//     if(close_file(fpin) == CY_ERR) return CY_ERR;
//     if(close_file(fpout) == CY_ERR) return CY_ERR;
//     return CY_OK;


// FILE *fp, *fp2; fp = fopen("prv.key", "rb"); fp2 = fopen("pub.key", "rb");
//     mpz_t pubkey[2], prvkey[2], msg[5]; mpz_inits(prvkey[0], prvkey[1], pubkey[0], pubkey[1], msg[0], msg[1], msg[2], msg[3], msg[4], NULL);
//     gmp_fscanf(fp, "%Zd\n%Zd", prvkey[0], prvkey[1]);
//     gmp_fscanf(fp2, "%Zd\n%Zd", pubkey[0], pubkey[1]);
//     gmp_printf("(private)\nd = %Zd\nn = %Zd\n\n", prvkey[0], prvkey[1]);
//     gmp_printf("(public)\ne = %Zd\nn = %Zd\n", pubkey[0], pubkey[1]);
//     // cy_rsa_key_imp("prv.key", prvkey);
//     char c[] = "hmida";
//     char pl[5];
//     gmp_printf("plaintext:\n\t%s\n\n", c);
//     gmp_printf("we devide it:\n\t");
//     for (uint8_t i = 0; i < 5; i++)
//     {
//         gmp_printf("%c\t", c[i]);  
//     }
//     printf("\n\t");
//     for (uint8_t i = 0; i < 5; i++)
//     {
//         gmp_printf("%d\t", c[i]);  
//     }
//     printf("\n\n");
    
//     for (int i = 0; i < 5; i++)
//     {
//         cy_rsa_encryption(c[i], prvkey, msg[i]);
//         gmp_printf("rsa(%c,(d,n)) = %Zd = c%d\n\n", c[i], msg[i], i+1);
//     }

//     for (int i = 0; i < 5; i++)
//     {
//         cy_rsa_decryption(msg[i], pubkey, &pl[i]);

//         gmp_printf("rsa(c%d = \n%Zd,y\n(e,n)) = %c\n\n", i+1, msg[i], pl[i]);
//     }


//  test RSA-----------------------

// #include <stdio.h>
// #include <string.h>
// #include <cypher.h>

// static void print_u128_hex(__uint128_t x) {
//     for (long unsigned int i = 0; i < sizeof(__uint128_t); i++) 
//         printf("%x\t", (uint8_t) ((x >> (sizeof(__uint128_t) - i - 1) * 8) & 0xFF));
// }

// static void print_u32_hex(uint32_t x) 
// {
//     for (long unsigned int i = 0; i < sizeof(uint32_t); i++)
//         printf("%x\t", (x >> (sizeof(uint32_t) - i - 1)*8)&0xFF);
// }

// static void print_u128_matrix(uint8_t tab[4][4])
// {
//     for (int8_t i = 3; i >= 0; i--) printf("%x\t%x\t%x\t%x\n\n", tab[i][0], tab[i][1], tab[i][2], tab[i][3]);
// }

// static const uint8_t CY_AES_MIXCOL_MAT[4][4] = 
// {
//     {0x02, 0x03, 0x01, 0x01},
//     {0x01, 0x02, 0x03, 0x01},
//     {0x01, 0x01, 0x02, 0x03},
//     {0x03, 0x01, 0x01, 0x02}
// };

// static const uint8_t CY_AES_SBOX[16][16] = 
// {
//     {0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76},
//     {0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0},
//     {0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15},
//     {0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75},
//     {0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84},
//     {0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF},
//     {0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8},
//     {0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2},
//     {0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73},
//     {0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB},
//     {0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79},
//     {0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08},
//     {0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A},
//     {0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E},
//     {0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF},
//     {0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16}
// };

// int main(void)
// {
//     __uint128_t key; __uint128_t msg = 0, cy_msg = 0;
//     uint32_t expandkey[44]; uint8_t state[4][4];
//     cy_aes_key_imp("aes.key", &key);
//     char hmida[] = "hello hmida fink";
//     for (size_t i = 0; i < 16; i++)
//     {
//         msg |=((__uint128_t)(hmida[i])) << (15 - i)*8;
//     }
    
//     printf("Message\n\n");
//     print_u128_hex(msg);
//     printf("\n\n");
//     printf("Generated key\n\n");
//     print_u128_hex(key);
//     printf("\n\n");
//     printf("Message from Stream to Block\n\n");
//     cy_aes_from_128_to_4by4(msg, state);
//     print_u128_matrix(state);
//     cy_aes_key_expansion(key, expandkey);
//     printf("Round(0)\n\nkey(0):\t");
//     for (size_t j = 0; j < 4; j++)
//     {
//         print_u32_hex(expandkey[3 - j]);
//     }
//     printf("\n\n");
//     cy_aes_add_round_key(expandkey, state);
//     print_u128_matrix(state);
//     printf("\n\n");

//     for (uint8_t i = 1; i < 10; i++)
//     {
//         printf("Round(%u)\n\nkey(%u):\t", i, i);
//         for (size_t j = 0; j < 4; j++)
//         {
//             print_u32_hex(expandkey[(i * 4) + 3 - j]);
//         }
//         printf("\n\n");
//         cy_aes_substitute_bytes(CY_AES_SBOX, state);
//         printf("Substitute Bytes\n\n");
//         print_u128_matrix(state);
//         cy_aes_shift_rows(state);
//         printf("Shift Rows\n\n");
//         print_u128_matrix(state);
//         cy_aes_mix_columns(CY_AES_MIXCOL_MAT, state);
//         printf("Mix Columns\n\n");
//         print_u128_matrix(state);
//         cy_aes_add_round_key(expandkey + (i * 4), state);
//         printf("Add Round Key\n\n");
//         print_u128_matrix(state);
//         printf("\n\n");
//     }
//     printf("Round(10)\n\nkey(10):\t");
//     for (size_t j = 0; j < 4; j++)
//     {
//         print_u32_hex(expandkey[40 + j]);
//     }
//     printf("\n\n");
//     cy_aes_substitute_bytes(CY_AES_SBOX, state);
//     printf("Substitute Bytes\n\n");
//     print_u128_matrix(state);
//     cy_aes_shift_rows(state);
//     printf("Shift Rows\n\n");
//     print_u128_matrix(state);
//     cy_aes_add_round_key(expandkey + 40, state);
//     printf("Add Round Key\n\n");
//     print_u128_matrix(state);
//     cy_aes_from_4by4_to_128(state, &cy_msg);
//     printf("Cypher Message\n\n");
//     print_u128_hex(cy_msg);
//     printf("\n\n");

//     return 0;
// }

// int main(void)
// {
//     // __uint128_t key; uint8_t tab[4][4];
//     // uint32_t w[44];
//     __uint128_t msg = 0, key, cy_msg = 0;
//     uint32_t expandkey[44];
//     uint8_t state[4][4];
//     char hmida[] = "hello hmida fink";
//     for (size_t i = 0; i < 16; i++)
//     {
//         msg |=((__uint128_t)(hmida[i])) << (15 - i)*8;
//     }
    
//     printf("Message\n\n");
//     print_u128_hex(msg);
//     printf("\n\n");
//     cy_aes_key_gen(&key);
//     printf("Generated key\n\n");
//     print_u128_hex(key);
//     printf("\n\n");
//     printf("Message from Stream to Block\n\n");
//     cy_aes_from_128_to_4by4(msg, state);
//     print_u128_matrix(state);
//     cy_aes_key_expansion(key, expandkey);
//     printf("Round(0)\n\nkey(0):\t");
//     for (size_t j = 0; j < 4; j++)
//     {
//         print_u32_hex(expandkey[3 - j]);
//     }
//     printf("\n\n");
//     cy_aes_add_round_key(expandkey, state);
//     print_u128_matrix(state);
//     printf("\n\n");

//     for (uint8_t i = 1; i < 10; i++)
//     {
//         printf("Round(%u)\n\nkey(%u):\t", i, i);
//         for (size_t j = 0; j < 4; j++)
//         {
//             print_u32_hex(expandkey[(i * 4) + 3 - j]);
//         }
//         printf("\n\n");
//         cy_aes_substitute_bytes(CY_AES_SBOX, state);
//         printf("Substitute Bytes\n\n");
//         print_u128_matrix(state);
//         cy_aes_shift_rows(state);
//         printf("Shift Rows\n\n");
//         print_u128_matrix(state);
//         cy_aes_mix_columns(CY_AES_MIXCOL_MAT, state);
//         printf("Mix Columns\n\n");
//         print_u128_matrix(state);
//         cy_aes_add_round_key(expandkey + (i * 4), state);
//         printf("Add Round Key\n\n");
//         print_u128_matrix(state);
//         printf("\n\n");
//     }
//     printf("Round(10)\n\nkey(10):\t");
//     for (size_t j = 0; j < 4; j++)
//     {
//         print_u32_hex(expandkey[40 + j]);
//     }
//     printf("\n\n");
//     cy_aes_substitute_bytes(CY_AES_SBOX, state);
//     printf("Substitute Bytes\n\n");
//     print_u128_matrix(state);
//     cy_aes_shift_rows(state);
//     printf("Shift Rows\n\n");
//     print_u128_matrix(state);
//     cy_aes_add_round_key(expandkey + 40, state);
//     printf("Add Round Key\n\n");
//     print_u128_matrix(state);
//     cy_aes_from_4by4_to_128(state, &cy_msg);
//     printf("Cypher Message\n\n");
//     print_u128_hex(cy_msg);
//     printf("\n\n");

//     for (size_t i = 0; i < 16; i++)
//     {
//         msg |=((__uint128_t)(hmida[i])) << (15 - i)*8;
//     }

//     printf("Message\n\n");
//     print_u128_hex(msg);
//     printf("\n\n");

//     msg = 0;
//     cy_aes_decryption(cy_msg, key, &msg);

//     printf("Decrypted Message\n\n");
//     print_u128_hex(msg);
//     printf("\n\n");
    
//     return 0;
// }