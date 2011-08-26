/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <cbfs.h>
#include <malloc.h>
#include <asm/byteorder.h>

CbfsResult file_cbfs_result;

const char *file_cbfs_error(void)
{
	switch (file_cbfs_result) {
	    case CBFS_SUCCESS:
		return "Success";
	    case CBFS_NOT_INITIALIZED:
		return "CBFS not initialized";
	    case CBFS_BAD_HEADER:
		return "Bad CBFS header";
	    case CBFS_BAD_FILE:
		return "Bad CBFS file";
	    case CBFS_FILE_NOT_FOUND:
		return "File not found";
	    default:
		return "Unknown";
	}
}

typedef struct CbfsFileHeader {
	u8 magic[8];
	u32 len;
	u32 type;
	u32 checksum;
	u32 offset;
} __attribute__((packed)) CbfsFileHeader;

typedef struct CbfsCacheNode {
	struct CbfsCacheNode *next;
	u32 type;
	void *data;
	u32 dataLength;
	char *name;
	u32 nameLength;
	u32 checksum;
} __attribute__((packed)) CbfsCacheNode;


static const u32 goodMagic = 0x4f524243;
static const u8 goodFileMagic[] = "LARCHIVE";


static int initialized;
static struct CbfsHeader cbfsHeader;
static CbfsCacheNode *fileCache;

/* Do endian conversion on the CBFS header structure. */
static void swap_header(CbfsHeader *dest, CbfsHeader *src)
{
	dest->magic = be32_to_cpu(src->magic);
	dest->version = be32_to_cpu(src->version);
	dest->romSize = be32_to_cpu(src->romSize);
	dest->bootBlockSize = be32_to_cpu(src->bootBlockSize);
	dest->align = be32_to_cpu(src->align);
	dest->offset = be32_to_cpu(src->offset);
}

/* Do endian conversion on a CBFS file header. */
static void swap_file_header(CbfsFileHeader *dest, CbfsFileHeader *src)
{
	memcpy(&dest->magic, &src->magic, sizeof(dest->magic));
	dest->len = be32_to_cpu(src->len);
	dest->type = be32_to_cpu(src->type);
	dest->checksum = be32_to_cpu(src->checksum);
	dest->offset = be32_to_cpu(src->offset);
}

/*
 * Given a starting position in memory, scan forward, bounded by a size, and
 * find the next valid CBFS file. No memory is allocated by this function. The
 * caller is responsible for allocating space for the new file structure.
 *
 * @param start		The location in memory to start from.
 * @param size		The size of the memory region to search.
 * @param align		The alignment boundaries to check on.
 * @param newNode	A pointer to the file structure to load.
 * @param used		A pointer to the count of of bytes scanned through,
 *			including the file if one is found.
 *
 * @return 1 if a file is found, 0 if one isn't.
 */
static int file_cbfs_next_file(u8 *start, u32 size, u32 align,
	CbfsCacheNode *newNode, u32 *used)
{
	CbfsFileHeader header;

	*used = 0;

	while (size >= align) {
		CbfsFileHeader *fileHeader = (CbfsFileHeader *)start;
		u32 nameLen;
		u32 step;

		/* Check if there's a file here. */
		if (memcmp(goodFileMagic, &(fileHeader->magic),
				sizeof(fileHeader->magic))) {
			*used += align;
			size -= align;
			start += align;
			continue;
		}

		swap_file_header(&header, fileHeader);
		if (header.offset < sizeof(CbfsFileHeader) ||
				header.offset > header.len) {
			file_cbfs_result = CBFS_BAD_FILE;
			return -1;
		}
		newNode->next = NULL;
		newNode->type = header.type;
		newNode->data = start + header.offset;
		newNode->dataLength = header.len;
		nameLen = header.offset - sizeof(CbfsFileHeader);
		newNode->name = (char *)fileHeader + sizeof(CbfsFileHeader);
		newNode->nameLength = nameLen;
		newNode->checksum = header.checksum;

		step = header.len;
		if (step % align)
			step = step + align - step % align;

		*used += step;
		return 1;
	}
	return 0;
}

/* Look through a CBFS instance and copy file metadata into regular memory. */
static void file_cbfs_fill_cache(u8 *start, u32 size, u32 align)
{
	CbfsCacheNode *cacheNode;
	CbfsCacheNode *newNode;
	CbfsCacheNode **cacheTail = &fileCache;

	/* Clear out old information. */
	cacheNode = fileCache;
	while (cacheNode) {
		CbfsCacheNode *oldNode = cacheNode;
		cacheNode = cacheNode->next;
		free(oldNode);
	}
	fileCache = NULL;

	while (size >= align) {
		int result;
		u32 used;

		newNode = (CbfsCacheNode *)malloc(sizeof(CbfsCacheNode));
		result = file_cbfs_next_file(start, size, align,
			newNode, &used);

		if (result < 0) {
			free(newNode);
			return;
		} else if (result == 0) {
			free(newNode);
			break;
		}
		*cacheTail = newNode;
		cacheTail = &newNode->next;

		size -= used;
		start += used;
	}
	file_cbfs_result = CBFS_SUCCESS;
}

/* Get the CBFS header out of the ROM and do endian conversion. */
static int file_cbfs_load_header(uintptr_t endOfRom, CbfsHeader *header)
{
	CbfsHeader *headerInRom;

	headerInRom = (CbfsHeader *)(uintptr_t)*(u32 *)(endOfRom - 3);
	swap_header(header, headerInRom);

	if (header->magic != goodMagic || header->offset >
			header->romSize - header->bootBlockSize) {
		file_cbfs_result = CBFS_BAD_HEADER;
		return 1;
	}
	return 0;
}

void file_cbfs_init(uintptr_t endOfRom)
{
	u8 *startOfRom;
	initialized = 0;

	if (file_cbfs_load_header(endOfRom, &cbfsHeader))
		return;

	startOfRom = (u8 *)(endOfRom + 1 - cbfsHeader.romSize);

	file_cbfs_fill_cache(startOfRom + cbfsHeader.offset,
			     cbfsHeader.romSize, cbfsHeader.align);
	if (file_cbfs_result == CBFS_SUCCESS)
		initialized = 1;
}

const CbfsHeader *file_cbfs_get_header(void)
{
	if (initialized) {
		file_cbfs_result = CBFS_SUCCESS;
		return &cbfsHeader;
	} else {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	}
}

CbfsFile file_cbfs_get_first(void)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return fileCache;
	}
}

void file_cbfs_get_next(CbfsFile *file)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		file = NULL;
		return;
	}

	if (*file)
		*file = (*file)->next;
	file_cbfs_result = CBFS_SUCCESS;
}

CbfsFile file_cbfs_find(const char *name)
{
	struct CbfsCacheNode *cacheNode = fileCache;

	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	}

	while (cacheNode) {
		if (!strcmp(name, cacheNode->name))
			break;
		cacheNode = cacheNode->next;
	}
	if (!cacheNode) {
		file_cbfs_result = CBFS_FILE_NOT_FOUND;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
	}
	return cacheNode;
}

CbfsFile file_cbfs_find_uncached(uintptr_t endOfRom, const char *name)
{
	u8 *start;
	u32 size;
	u32 align;
	static CbfsCacheNode node;

	if (file_cbfs_load_header(endOfRom, &cbfsHeader))
		return NULL;

	start = (u8 *)(endOfRom + 1 - cbfsHeader.romSize);
	size = cbfsHeader.romSize;
	align = cbfsHeader.align;

	while (size >= align) {
		int result;
		u32 used;

		result = file_cbfs_next_file(start, size, align, &node, &used);

		if (result < 0) {
			return NULL;
		} else if (result == 0) {
			break;
		}

		if (!strcmp(name, node.name)) {
			return &node;
		}

		size -= used;
		start += used;
	}
	file_cbfs_result = CBFS_FILE_NOT_FOUND;
	return NULL;
}

const char *file_cbfs_name(CbfsFile file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->name;
}

u32 file_cbfs_size(CbfsFile file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->dataLength;
}

u32 file_cbfs_type(CbfsFile file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->type;
}

long file_cbfs_read(CbfsFile file, void *buffer, unsigned long maxsize)
{
	u32 size;

	size = file->dataLength;
	if (maxsize && size > maxsize)
		size = maxsize;

	memcpy(buffer, file->data, size);

	file_cbfs_result = CBFS_SUCCESS;
	return size;
}
