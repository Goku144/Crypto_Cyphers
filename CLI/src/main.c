/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        /// TODO: check version and help
        fprintf(stderr, "usage: cypher <a> <b> (Dummy Template)\n");
        return 1;
    }
    srand((unsigned) time(NULL));
    // uint64_t n = 65;
    // Pair res = decompOdd(n);
    // printf("%"PRIu64" - 1 = 2^(%"PRIu64") * %"PRIu64"\n", n, res.power, res.odd);

    char *end;
    
    // printf("2^(%"PRIu64")%"PRIu64"\n", res.power, res.odd);
    // if(res == MRA_ERR) return 1;

    // if(res == composite)
    //     printf("the number is definitly not prime");
    // else
    //     printf("the number have good chance to be prime");
    uint64_t size = strtoull(argv[1], &end, 10), num = 1;
    printf("P(%"PRIu64") = 2\n", num++);
    for (uint64_t i = 3; i < size; i++)
    {
        if(EMRA(i, 20) == INCONCLUSIVE)
            printf("P(%"PRIu64") = %"PRIu64"\n", num++, i);
    }
    
    // for (size_t i = 0; i < 19; i++)
    // {
    //     printf("a = %"PRIu64"\n", bounded_a(strtoull(argv[1], &end, 10)));
    // }
    return 0;
}
