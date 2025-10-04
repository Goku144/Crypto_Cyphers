/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) 
{   
    (void) argc;
    CY_KEY key = {.owner=CY_NOT_OWNED,.size=0,.str=NULL};

    cypher(argv[1], &key, CY_encryption_EASCII, argv[2]);


    for (int i = 0; i < 256; i++) 
    {
        printf("%02X->%02X%s", i & 0xFF, key.str[i], (i%8==7) ? "\n" : " ");
    }
    
    cypher(argv[2], &key, CY_decryption_EASCII, argv[1]);

    for (int i = 0; i < 256; i++) 
    {
        printf("%02X->%02X%s", i & 0xFF, key.str[i], (i%8==7) ? "\n" : " ");
    }
    

    printf("\n");

    return 0;
}

// CY_KEY key = (CY_KEY) {.str=pair, .size=26, .owner=CY_OWNED};

// cypher(argv[1], &key, monoalphabetic, CY_DECRYPTION, argv[2]);

