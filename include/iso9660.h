/*
 * ISO9660 Filesystem by Adrian Cox adrian@humboldt.co.uk
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to u-boot
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ISO9660_H_
#define _ISO9660_H_

#include <asm/byteorder.h>


int file_iso9660_ls(const char *dir);
int iso9660_exists(const char *filename);
int iso9660_read_file(const char *filename, void *buf, int offset, int len);
int iso9660_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info);
void iso9660_close(void);

#endif /* _ISO9660_H_ */
