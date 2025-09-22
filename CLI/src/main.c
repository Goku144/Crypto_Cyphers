/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        /// TODO: check version and help
        fprintf(stderr, "usage: cypher <a> <b> (Dummy Template)\n");
        return 1;
    }
    char *end;
    Pair res = MRA(strtoull(argv[1], &end, 10));
    printf("2^(%"PRIu64")%"PRIu64"\n", res.power, res.odd);
    return 0;
}
