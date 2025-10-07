/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) 
{   
    (void) argc;

    // uint8_t str[] = {5,11};

    CY_KEY key = {.owner=CY_NOT_OWNED, .size=0, .str=NULL};

    // cy_key_linear_direct_generate(0, 255, &key);
    // CY_key_linear_rand_generated(0, 255, &key);
    

    cy_key_import(argv[3], &key);

    // printf("%zu", key.size);

    // for (size_t i = 0; i < key.size; i++) printf("%c ", key.str[i]);

    cypher(argv[2], &key, cy_crack_monoalpahbetic, argv[1]);
    // cy_key_export(key, argv[3]);
    // cy_key_export(key, argv[3]);
    free(key.str);

    return 0;
}

// CY_KEY key = (CY_KEY) {.str=pair, .size=26, .owner=CY_OWNED};

// cypher(argv[1], &key, monoalphabetic, CY_DECRYPTION, argv[2]);

