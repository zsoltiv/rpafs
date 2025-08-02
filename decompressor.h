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

#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include <inttypes.h>

int decompress_file_index(uint64_t compressed_file_index_sz,
                          uint8_t compressed_file_index[static compressed_file_index_sz],
                          uint64_t *decompressed_file_index_sz,
                          uint8_t *decompressed_file_index[static *decompressed_file_index_sz]);

#endif
