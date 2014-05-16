/*
 * iso9660.c
 *
 * ISO9660 Filesystem support by Adrian Cox adrian@humboldt.co.uk
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <exports.h>
#include <iso9660.h>
#include <asm/byteorder.h>
#include <part.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <linux/ctype.h>

static int iso9660_strcmp(const char *ondisk, const char *wanted)
{
    if (! *ondisk)
	return -1;
    
    while (*ondisk && *wanted)
    {
		if (tolower(*ondisk) != tolower(*wanted))
		    return -1;
		ondisk++;
		wanted++;
    }
    if (*ondisk && ! (*ondisk == ';' || *ondisk == '.'))
		return -1;
    
    return 0;
}

static block_dev_desc_t *cur_dev;
static disk_partition_t cur_part_info;

static int disk_read(__u32 block, __u32 nr_blocks, void *buf)
{
	if (!cur_dev || !cur_dev->block_read)
		return -1;

	return cur_dev->block_read(cur_dev->dev,
			cur_part_info.start + block, nr_blocks, buf);
}


static const u8 default_volume_desc[] = {1, 67, 68, 48, 48, 49, 1, 0};



int iso9660_set_blk_dev(block_dev_desc_t *dev_desc, disk_partition_t *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	cur_dev = dev_desc;
	cur_part_info = *info;

	/* Make sure it has a valid ISO9660 header */
	if (disk_read(16, 1, buffer) != 1) {
		cur_dev = NULL;
		return -1;
	}

	/* Check if it's actually an ISO9660 volume */
	if (memcmp(buffer, default_volume_desc, 8)) {
		printf("Failed to mount iso9660 filesystem\n");
		cur_dev = NULL;
		return -1;
	}

	printf("Mounted iso9660 filesystem\n");
	return 0;
}

int iso9660_register_device(block_dev_desc_t *dev_desc, int part_no)
{
	disk_partition_t info;

	/* First close any currently found ISO9660 filesystem */
	cur_dev = NULL;

	/* Read the partition table, if present */
	if (get_partition_info(dev_desc, part_no, &info)) {
		if (part_no != 0) {
			printf("** Partition %d not valid on device %d **\n",
					part_no, dev_desc->dev);
			return -1;
		}

		info.start = 0;
		info.size = dev_desc->lba;
		info.blksz = dev_desc->blksz;
		info.name[0] = 0;
		info.type[0] = 0;
		info.bootable = 0;
#ifdef CONFIG_PARTITION_UUIDS
		info.uuid[0] = 0;
#endif
	}

	return iso9660_set_blk_dev(dev_desc, &info);
}

/*
 * Get the first occurence of a directory delimiter ('/' or '\') in a string.
 * Return index into string if found, -1 otherwise.
 */
static int dirdelim(const char *str)
{
	const char *start = str;

	while (*str != '\0') {
		if (*str == '/' || *str == '\\')
			return str - start;
		str++;
	}
	return -1;
}

#define NOT_FOUND -1
#define FOUND_FILE 0
#define FOUND_DIR 1

static int iso9660_find_entry(u32 directory, u32 length, const char *filename,
	u32 *found_start, u32 *found_size) {	

	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, cur_dev->blksz);
    u32 i;

    for(i = 0; i < length; i += cur_dev->blksz, directory++)
    {
		int j = 0;
		if (disk_read(directory, 1, buffer) != 1)
		    return NOT_FOUND;
		while(j < cur_dev->blksz)
		{
		    u8 *entry = buffer + j;
		    u32 file_start;
		    u32 file_size;
		    u8 namelen;
		    char ondisk[65];
		    if (! entry[0])
				break;
		    j += entry[0];
		    namelen = entry[32];
		    if (namelen > 64)
		    	namelen = 64;
		    memcpy(ondisk, entry + 33, namelen);
		    ondisk[namelen] = 0;
		    file_start = (entry[5] << 24) | (entry[4] << 16) |
				(entry[3] << 8) | entry[2];
		    file_size = (entry[13] << 24) | (entry[12] << 16) |
				(entry[11] << 8) | entry[10];

			if (! iso9660_strcmp(ondisk, filename)) {
				*found_start = file_start;
				*found_size = file_size;
				if (entry[25] & 0x02)
					return FOUND_DIR;
				else
					return FOUND_FILE;
			}
		}
    }
    return -1;

}

static int iso9660_read_directory(u32 directory, u32 length)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, cur_dev->blksz);
    u32 i;
    for(i = 0; i < length; i += cur_dev->blksz, directory++)
    {
		int j = 0;
		if (disk_read(directory, 1, buffer) != 1)
		    return -1;
		while(j < cur_dev->blksz)
		{
		    u8 *entry = buffer + j;
		    u32 file_start;
		    u32 file_size;
		    u8 namelen;
		    char ondisk[65];
		    if (! entry[0])
				break;
		    j += entry[0];
		    namelen = entry[32];
		    if (namelen > 64)
		    	namelen = 64;
		    memcpy(ondisk, entry + 33, namelen);
		    ondisk[namelen] = 0;
		    file_start = (entry[5] << 24) | (entry[4] << 16) |
				(entry[3] << 8) | entry[2];
		    file_size = (entry[13] << 24) | (entry[12] << 16) |
				(entry[11] << 8) | entry[10];


			printf("%-16s %4d %8d\n", ondisk, file_start, file_size);
		}
    }
    return 0;
}

static int iso9660_scan(const char *dir, u32 *start, u32 *len) {
    unsigned char *root;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, cur_dev->blksz);

	/* Make sure it has a valid ISO9660 header */
	if (disk_read(16, 1, buffer) != 1) {
		return -1;
	}

	root = buffer + 156;
    *start = (root[5] << 24) | (root[4] << 16) |
	    (root[3] << 8) | root[2];
    *len = (root[13] << 24) | (root[12] << 16) |
	    (root[11] << 8) | root[10];

	if (!dir || strlen(dir) == 0)
		return FOUND_DIR;
	else {

		for(;;) {
			int dirpos = dirdelim(dir);
			if (dirpos < 0) {
				/* Final element of path, or empty element */
				if (strlen(dir) > 0)
					return iso9660_find_entry(*start, *len, dir, start, len);
				else
					return FOUND_DIR;
			} else {
				char name[64];
				if (dirpos > 64)
					return -1;
				memcpy(name, dir, dirpos);
				name[dirpos] = 0;
				dir += dirpos + 1;

				/* Don't scan empty string at root directory */
				if (dirpos > 0) {
					int result = iso9660_find_entry(*start, *len, name, start, len);

					if (result != FOUND_DIR)
						return result;
				}


			}

		}

	}

}



int file_iso9660_ls(const char *dir)
{	
  	u32 start, len;
  	int result = iso9660_scan(dir, &start, &len);
  	if (result == FOUND_DIR)
  		return iso9660_read_directory(start, len);
  	else if (result == FOUND_FILE) {
  		printf("%s  %u %u\n", dir, start, len);
  		return 0;
  	} else {
  		printf("Path not found\n");
  		return -1;
  	}

}

int iso9660_exists(const char *filename)
{
  	u32 start, len;
  	int result = iso9660_scan(filename, &start, &len);
  	if (result >= 0)
  		return 0;
  	else
  		return result;
}


int iso9660_read_file(const char *filename, void *buf, int offset, int readlen)
{
  	u32 start, len;
  	int result = iso9660_scan(filename, &start, &len);
  	if (result == FOUND_DIR){
  		printf("Path is a directory\n");
  		return -1;
  	} else if (result == FOUND_FILE) {
  		u32 block;
		ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, cur_dev->blksz);

  		// 0 == whole file
  		if (readlen == 0 || readlen + offset > len)
  			readlen = len - offset;

  		block = offset / cur_dev->blksz;
  		offset -= (block * cur_dev->blksz);
  		block += start;
  		while(readlen > 0) {
  			u32 bytes = cur_dev->blksz - offset;
  			if (bytes > readlen)
  				bytes = readlen;

  			if (disk_read(block, 1, buffer) != 1)
  				return -1;

  			memcpy(buf, buffer + offset, bytes);

  			readlen -= bytes;
  			buf += bytes;
  			block++;
  			offset = 0;
  		}

  		return 0;
  	} else {
  		printf("Path not found\n");
  		return -1;
  	}

}

void iso9660_close(void)
{
}
