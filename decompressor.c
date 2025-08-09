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
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

#include "decompressor.h"

// 1MiB for decode chunks
#define DECODE_CHUNK UINT32_C(1048576)

int decompress_file_index(uint64_t compressed_file_index_sz,
                          uint8_t compressed_file_index[static compressed_file_index_sz],
                          uint64_t *decompressed_file_index_sz,
                          uint8_t *decompressed_file_index[static *decompressed_file_index_sz])
{
    z_stream strm = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = compressed_file_index_sz,
        .next_in = compressed_file_index,
    };
    *decompressed_file_index_sz = 0;
    *decompressed_file_index = NULL;
    int ret = inflateInit(&strm);
    if (ret != Z_OK) {
        fprintf(stderr, "Failed to init zlib\n");
        return ret;
    }
    do {
        *decompressed_file_index_sz += DECODE_CHUNK;
        *decompressed_file_index = realloc(*decompressed_file_index, *decompressed_file_index_sz);
        strm.avail_out = DECODE_CHUNK;
        strm.next_out = (*decompressed_file_index) + (*decompressed_file_index_sz) - DECODE_CHUNK;
        do {
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                    /* fallthrough */
                case Z_DATA_ERROR:
                    inflateEnd(&strm);
                    free(*decompressed_file_index);
                    fprintf(stderr, "Z_DATA_ERROR\n");
                    return ret;
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    free(*decompressed_file_index);
                    fprintf(stderr, "Z_MEM_ERROR\n");
                    return ret;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    *decompressed_file_index_sz = strm.total_out;
    *decompressed_file_index = realloc(*decompressed_file_index, *decompressed_file_index_sz);
    inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
