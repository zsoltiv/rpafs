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

#ifndef FUSE_H
#define FUSE_H

#define FUSE_USE_VERSION 31
#include <fuse.h>

void *rpa_init(struct fuse_conn_info *ci, struct fuse_config *cfg);
int rpa_getattr(const char *path, struct stat *st, struct fuse_file_info *fi);
int rpa_readdir(const char *path,
                void *data,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *ffi,
                enum fuse_readdir_flags flags);
int rpa_open(const char *path, struct fuse_file_info *fi);
int rpa_read(const char *path,
             char *buf,
             size_t sz,
             off_t offset,
             struct fuse_file_info *fi);

#endif
