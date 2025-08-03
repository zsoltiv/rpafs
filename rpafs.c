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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <stddef.h>

#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <zlib.h>

#include "decompressor.h"
#include "unpickle.h"
#include "fs.h"
#include "fuse.h"

extern struct rpa_node root;
extern blksize_t archive_blocksize;
extern int rpafd;

struct fuse_operations rpa_ops = {
    .init = rpa_init,
    .getattr = rpa_getattr,
    .readdir = rpa_readdir,
    .open = rpa_open,
    .read = rpa_read,
};

static struct rpa_options {
    const char *archive_name;
} rpa_opts;

#define OPTION(t, p) { t, offsetof(struct rpa_options, p), 1 }
struct fuse_opt fuse_opts[] = {
    OPTION("--archive=%s", archive_name),
    FUSE_OPT_END,
};

#define RPA3_HEADER_SZ 34

static const char RPA3_MAGIC[] = {'R', 'P', 'A', '-', '3', '.', '0'};

int main(int argc, char **argv)
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&args, &rpa_opts, fuse_opts, NULL) < 0) {
        fprintf(stderr, "Usage: rpafs --archive=</path/to/archive.rpa> </path/to/mount/point>\n");
        return EXIT_FAILURE;
    }

    if (!rpa_opts.archive_name) {
        fprintf(stderr, "No archive name supplied\n");
        return EXIT_FAILURE;
    }

    int ret;
    if ((rpafd = open(rpa_opts.archive_name, O_RDONLY)) < 0) {
        fprintf(stderr, "Failed to open rpa file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(rpafd, &st) < 0) {
        fprintf(stderr, "Failed to stat %s: %s", rpa_opts.archive_name, strerror(errno));
        goto err_stat;
    }
    archive_blocksize = st.st_blksize;
    off_t archive_sz = st.st_size;
    if (archive_sz < RPA3_HEADER_SZ) {
        fprintf(stderr, "Missing RPA-3.0 header\n");
        goto err_stat;
    }
    char header[RPA3_HEADER_SZ + 1] = {0};
    if ((ret = read(rpafd, &header, RPA3_HEADER_SZ)) < RPA3_HEADER_SZ) {
        if (ret < 0) {
            fprintf(stderr, "Failed to read archive header due to error: %s\n", strerror(errno));
        } else {
            fprintf(stderr, "Failed to read archive header, read %d bytes\n", ret);
        }
        goto err_stat;
    }
    if (memcmp(header, RPA3_MAGIC, sizeof(RPA3_MAGIC))) {
        fprintf(stderr, "Wrong header\n");
        goto err_stat;
    }

    char *prev = NULL;
    uint64_t compressed_index_offset = (uint64_t)strtoull(&header[sizeof(RPA3_MAGIC) + 2], &prev, 16);
    uint64_t xor_key = (uint64_t)strtoull(prev + 1, NULL, 16);
    //printf("encoded_index_offset=%"PRIx64" xor_key=%"PRIx64"\n", compressed_index_offset, xor_key);
    uint64_t compressed_index_sz = archive_sz - compressed_index_offset;

    if (lseek(rpafd, compressed_index_offset, SEEK_SET) < 0) {
        fprintf(stderr, "lseek() failed: %s\n", strerror(errno));
        goto err_stat;
    }
    uint8_t *compressed_index = calloc(compressed_index_sz, 1);
    if (read(rpafd, compressed_index, compressed_index_sz) < 0) {
        fprintf(stderr, "Failed to read file index: %s\n", strerror(errno));
        goto err_index;
    }
    uint64_t file_index_sz = 0;
    uint8_t *file_index = NULL;
    ret = decompress_file_index(compressed_index_sz, compressed_index, &file_index_sz, &file_index);
    if (ret != Z_OK) {
        fprintf(stderr, "Error decompressing the archive index\n");
        goto err_index;
    }
    free(compressed_index);

    unpickle_index(file_index_sz, file_index, xor_key, &root);
    free(file_index);

    ret = fuse_main(args.argc, args.argv, &rpa_ops, &root);
    fuse_opt_free_args(&args);

    return EXIT_SUCCESS;

err_index:
    free(compressed_index);
err_stat:
    close(rpafd);
    return EXIT_FAILURE;
}
