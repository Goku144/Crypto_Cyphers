/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) 
{   
    (void) argc;
    uint8_t str[] = "vtkdjqxlsioyafphbewgrcnumz";
    CY_KEY key = {.owner=CY_OWNED,.size=26,.str=str};

    cypher(argv[1], &key, CY_decryption_monoalphabetic, argv[2]);

    printf("the key: ");
    for (size_t i = 0; i < key.size; i++) printf("%c", key.str[i]);
    printf("\n");

    return 0;
}

// CY_KEY key = (CY_KEY) {.str=pair, .size=26, .owner=CY_OWNED};

// cypher(argv[1], &key, monoalphabetic, CY_DECRYPTION, argv[2]);

