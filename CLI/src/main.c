/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>



int main(int argc, const char *argv[]) 
{   
    monoalphabeticCypher(argv[1], NULL, DECRYPTION, argv[2]);
    return 0;
}

// for (uint8_t i = 0; i < 26; i++)
//         {
//             printf("%c", key[i]);
//             if (i != 25)
//                 printf(", "); 
//         }
//         printf("\n\n");


// NUMBER_FLAG out;
//     for (size_t i = 3; i < 1000; i++)
//     {
//         EMRA(i,20,&out);
//         if(out == INCONCLUSIVE)
//             printf("%zu is prime\n", i);
//         else
//             printf("%zu is not prime\n", i);
//     }