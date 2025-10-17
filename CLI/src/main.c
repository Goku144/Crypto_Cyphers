/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) 
{   
    (void) argc;
    uint8_t pair[] = "qwertzuiopasdfghjklyxcvbnm";
    CY_KEY key = (CY_KEY) {.str=pair, .size=26, .owner=CY_OWNED};

    cypher(argv[1], &key, monoalphabetic, CY_DECRYPTION, argv[2]);

    printf("the key = ");
    printf("%s", key.str);
    printf("\n");
    
    return 0;
}

