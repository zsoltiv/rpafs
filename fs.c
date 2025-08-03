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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FUSE_USE_VERSION 31
#include <fuse.h>

#include "fs.h"

const char *next_slash(const char *p)
{
    const char *q = p;
    while (*q && (*q) != '/') q++;
    return q;
}

void add_node_to_tree(struct rpa_node *root, const char *path, uint32_t offset, uint32_t size)
{
    struct rpa_node *node = root;
    const char *p = path;
    size_t path_len = strlen(path);
    while (p < path + path_len) {
        const char *q = next_slash(p);
        bool found = false;
        for (unsigned i = 0; i < node->node.dir.nb_entries; i++) {
            struct rpa_node *e = node->node.dir.entries[i];
            if (strncmp(e->name, p, q - p) == 0) {
                found = true;
                node = e;
                break;
            }
        }
        if (!found) {
            node->node.dir.entries = realloc(node->node.dir.entries, (node->node.dir.nb_entries + 1) * sizeof(struct rpa_entry *));
            struct rpa_node *new_node = calloc(sizeof(struct rpa_node), 1);
            new_node->is_dir = true; // everything starts off as a directory
            new_node->name = calloc(q - p + 1, 1);
            new_node->node.dir.nb_entries = 0;
            memcpy(new_node->name, p, q - p);
            node->node.dir.entries[node->node.dir.nb_entries] = new_node;
            node->node.dir.nb_entries++;
            node = new_node;
        }
        if(!(*q)) {
            node->is_dir = false;
            node->node.file.offset = offset;
            node->node.file.size = size;
            p = q;
        } else {
            p = q + 1; // next component
        }
    }
}

// TODO
struct rpa_node *rpa_find_node(struct rpa_node *root, const char *path)
{
    printf("path=%s\n", path);
    size_t path_len = strlen(path);
    const char *p = path + path_len - 1;
    while (p > path && *p != '/') p--;
    p++;
    size_t target_name_len = path + path_len - p;
    char *target_name = calloc(target_name_len + 1, 1);
    memcpy(target_name, p, target_name_len);
    printf("final component=%s\n", target_name);
    p = path;
    unsigned target_name_occurences = 0;
    while (p < path + path_len) {
        const char *q = next_slash(p);
        size_t component_len = q - p;
        size_t chars_to_compare = target_name_len > component_len ? component_len : target_name_len;
        if (memcmp(target_name, p, chars_to_compare) == 0)
            target_name_occurences++;
        p = q;
        if (*p) p++; // if it's a slash, go to the next component
    }

    p = path;
    struct rpa_node *node = root;
    size_t found_target_names = 0;

    while (p < path + path_len) {
        const char *q = next_slash(p);
        size_t component_len = q - p;
        size_t chars_to_compare = target_name_len > component_len ? component_len : target_name_len;
        if (node->is_dir) {
            for (unsigned i = 0; i < node->node.dir.nb_entries; i++) {
                struct rpa_node *e = node->node.dir.entries[i];
                if (strncmp(p, e->name, component_len) == 0) {

                    break;
                }
            }
        } else {
            // regular files can only be the final component
            break;
        }
        if (memcmp(target_name, p, chars_to_compare) == 0)
            found_target_names++;
    }

    printf("target_name_occurences=%u\n", target_name_occurences);

    return NULL;
}
