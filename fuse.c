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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#define FUSE_USE_VERSION 31
#include <fuse.h>

#include "fs.h"
#include "fuse.h"

struct rpa_node root = {
    .is_dir = true,
    .node.dir.nb_entries = 0,
    .node.dir.entries = NULL,
};
blksize_t archive_blocksize = 0;
int rpafd;

static struct rpa_node *find_node(struct rpa_node *root, const char *path)
{
    if (strncmp("/", path, strlen(path)) == 0) {
        return root;
    }
    struct rpa_node *node = root;
    size_t path_len = strlen(path);
    const char *p = path;
    if ((*p) == '/') p++;
    while (p < path + path_len) {
        const char *q = next_slash(p);
        size_t component_len = q - p;
        bool found = false;
        for (unsigned i = 0; i < node->node.dir.nb_entries; i++) {
            struct rpa_node *e = node->node.dir.entries[i];
            size_t node_name_len = strlen(e->name);
            if (component_len == node_name_len && strncmp(e->name, p, component_len) == 0) {
                node = e;
                found = true;
                break;
            }
        }
        if (!found)
            return NULL;
        if (*q) {
            p = q + 1;
        } else {
            p = q;
        }
    }

    return node;
}

void *rpa_init(struct fuse_conn_info *ci, struct fuse_config *cfg)
{
    return &root;
}

int rpa_getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    struct rpa_node *node = find_node(&root, path);
    if (node)
        st->st_blksize = archive_blocksize;
    else {
        fprintf(stderr, "not found %s\n", path);
        return -ENOENT;
    }
    st->st_nlink = 1;
    if (!node->is_dir) {
        st->st_size = node->node.file.size;
        st->st_mode = 0444;
        st->st_mode = S_IFREG;
    } else {
        st->st_mode = 0555;
        st->st_mode = S_IFDIR;
    }

    return 0;
}

int rpa_readdir(const char *path,
                void *data,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *ffi,
                enum fuse_readdir_flags flags)
{
    filler(data, ".", NULL, 0, 0);
    filler(data, "..", NULL, 0, 0);

    struct rpa_node *node = find_node(&root, path);
    for (unsigned i = 0; i < node->node.dir.nb_entries; i++) {
        struct rpa_node *e = node->node.dir.entries[i];
        filler(data, e->name, NULL, 0, 0);
    }

    return 0;
}

int rpa_open(const char *path, struct fuse_file_info *fi)
{
    struct rpa_node *node = find_node(&root, path);
    if (!node)
        return -ENOENT;
    if (!node->is_dir) {
        if ((fi->flags & O_RDONLY) != O_RDONLY)
            return -EACCES;
        return 0;
    }
    return -ENOENT;
}

int rpa_read(const char *path,
             char *buf,
             size_t sz,
             off_t offset,
             struct fuse_file_info *fi)
{
    struct rpa_node *node = find_node(&root, path);
    uint32_t asset_offset = node->node.file.offset;
    uint32_t asset_size = node->node.file.size;
    if (offset + sz > asset_size) {
        sz = asset_size - offset;
    }
    if (lseek(rpafd, asset_offset + offset, SEEK_SET) < 0)
        return -errno;
    int ret = 0;
    do {
         ret = read(rpafd, buf + ret, sz - ret);
         if (ret < 0)
             return -errno;
    } while (ret < sz);
    return ret;
}
