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
#include <inttypes.h>

struct rpa_node {
    char *name;
    union {
        struct {
            uint32_t offset, size;
        } file;
        struct {
            struct rpa_node **entries;
            int nb_entries;
        } dir;
    } node;
    bool is_dir;
};

struct rpa_node *rpa_find_node(struct rpa_node *root, const char *path);
void add_node_to_tree(struct rpa_node *root, const char *path, uint32_t offset, uint32_t size);

#endif
