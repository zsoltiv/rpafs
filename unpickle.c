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

void unpickle_index(const uint64_t file_index_sz,
                    const uint8_t file_index[static file_index_sz],
                    uint32_t key,
                    struct rpa_node *root)
{
    const uint8_t *p = file_index;
    // preallocate 10k entries, will reallocate later
    unsigned nb_entries = 10000, entry_idx = 0;
    enum next_binint next_binint = NEXT_OFFSET;
    uint32_t val = 0;
    char *path = NULL;
    uint32_t size, offset;
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
                        entry_idx++;
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
                p += 2 + val;
                break;
            default:
                p++;
        }
    }
}
