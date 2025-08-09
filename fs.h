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

#ifndef FS_H
#define FS_H

#include <stdbool.h>
#define _FILE_OFFSET_BITS 64 // refer to the manual page `feature_test_macros`
#include <sys/types.h>
#include <inttypes.h>

struct rpa_node {
    /*
     * XXX this is not NULL-terminated as we
     * do length comparisons and memcmp
     */
    char *name;
    union {
        struct {
            off_t offset, size;
        } file;
        struct {
            struct rpa_node **entries;
            uint64_t nb_entries;
        } dir;
    } node;
    size_t namelen;
    bool is_dir;
};

const char *next_slash(const char *p);

struct rpa_node *rpa_find_node(struct rpa_node *root, const char *path);
void add_node_to_tree(struct rpa_node *root, const char *path, off_t offset, off_t size);

#endif
