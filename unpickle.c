/*
 *    This file is part of rpafs.
 *
 *    rpafs is free software: you can redistribute it and/or modify it under
 *    the terms of the GNU General Public License as published by the
 *    Free Software Foundation, either version 3 of the License,
 *    or (at your option) any later version.
 *
 *    rpafs is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the
 *    GNU General Public License along with rpafs.
 *    If not, see <https://www.gnu.org/licenses/>.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>

#include "fs.h"
#include "unpickle.h"

// Relevant opcodes taken from CPython
enum opcode {
    BININT           =    'J',
    BININT1          =    'K',
    LONG             =    'L',
    BININT2          =    'M',
    STRING           =    'S',
    BINSTRING        =    'T',
    SHORT_BINSTRING  =    'U',
    UNICODE          =    'V',
    BINUNICODE       =    'X',
    BINFLOAT         =    'G',
    LONG1            = '\x8a',
    LONG4            = '\x8b',
    BINUNICODE8      = '\x8d',
    SHORT_BINUNICODE = '\x8c',
};

enum next_binint {
    NEXT_OFFSET = 0,
    NEXT_SIZE = 1,
};

static off_t unpickle_long1(const uint8_t width, const uint8_t bytes[static width])
{
    const uint8_t *last_byte = bytes + width - 1;
    bool is_signed = *last_byte >= 0x80;
    uint8_t insignificant = is_signed ? 0xFF : 0x0;
    off_t accum = 0;
    for (uint8_t i = 0; i < width; i++) {
        if (bytes[i] == insignificant)
            break;
        accum |= ((off_t)bytes[i]) << (i * 8);
    }

    if (is_signed) {
        accum -= 1;
        accum = ~accum;
    }

    return accum;
}

void unpickle_index(const uint64_t file_index_sz,
                    const uint8_t file_index[static file_index_sz],
                    uint32_t key,
                    struct rpa_node *root)
{
    const uint8_t *p = file_index;
    enum next_binint next_binint = NEXT_OFFSET;
    char *path = NULL;
    off_t num_width, val, size, offset;
    bool is_signed;
    while (p < file_index + file_index_sz) {
        switch(*p) {
            case BININT:
                val = (((*(p + 1)) <<  0) |
                       ((*(p + 2)) <<  8) |
                       ((*(p + 3)) << 16) |
                       ((*(p + 4)) << 24)) ^ key;
                switch (next_binint) {
                    case NEXT_OFFSET:
                        offset = val;
                        break;
                    case NEXT_SIZE:
                        size = val;
                        add_node_to_tree(root, path, offset, size);
                        break;
                }
                next_binint = !next_binint;
                p += 5;
                break;
            case (uint8_t) SHORT_BINUNICODE:
                val = (*(p + 1));
                if (path) {
                    free(path);
                    path = NULL;
                }
                path = calloc(val + 1, 1);
                memcpy(path, p + 2, val);
                next_binint = NEXT_OFFSET;
                p += 2 + val;
                break;
            case (uint8_t) LONG1:
                num_width = *(p + 1);
                val = unpickle_long1(num_width, p + 2) ^ key;
                switch (next_binint) {
                    case NEXT_OFFSET:
                        offset = val;
                        break;
                    case NEXT_SIZE:
                        size = val;
                        add_node_to_tree(root, path, offset, size);
                        break;
                }
                p += 2 + num_width;
                next_binint = !next_binint;
                break;
            case LONG:
            case BININT1:
            case BININT2:
            case STRING:
            case BINSTRING:
            case UNICODE:
            case BINUNICODE:
            case (uint8_t) LONG4:
            case (uint8_t) BINUNICODE8:
            case SHORT_BINSTRING:
                fprintf(stderr, "Unhandled opcode %x\n", *p);
            default:
                p++;
        }
    }
}
