/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Jebbari Marouane
 */

#include <stdio.h>
#include <string.h>
#include <cypher.h>

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        // TODO: check version and help
        fprintf(stderr, "usage: %s <a> <b>\n", argv[0]);
        return 1;
    }
    char *end;
    printf("%llu\n", mulModInverse(strtoull(argv[1], &end, 10), strtoull(argv[2], &end, 10)));
    return 0;
}
