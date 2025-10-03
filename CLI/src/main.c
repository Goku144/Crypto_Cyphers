/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) 
{   
    uint8_t keystr[] = {2,1};
    CY_KEY key = {.owner=CY_OWNED,.size=2,.str=keystr};
    cypher(argv[1], &key, CY_decryption_caesar, argv[2]);

    return 0;
}

// CY_KEY key = (CY_KEY) {.str=pair, .size=26, .owner=CY_OWNED};

// cypher(argv[1], &key, monoalphabetic, CY_DECRYPTION, argv[2]);

