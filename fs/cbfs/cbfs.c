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

const char *
file_cbfs_error(void)
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
static struct CbfsHeader header;
static CbfsCacheNode *fileCache;

static void
swap_header(CbfsHeader *dest, CbfsHeader *src)
{
	dest->magic = be32_to_cpu(src->magic);
	dest->version = be32_to_cpu(src->version);
	dest->romSize = be32_to_cpu(src->romSize);
	dest->bootBlockSize = be32_to_cpu(src->bootBlockSize);
	dest->align = be32_to_cpu(src->align);
	dest->offset = be32_to_cpu(src->offset);
}

static void
swap_file_header(CbfsFileHeader *dest, CbfsFileHeader *src)
{
	memcpy(&dest->magic, &src->magic, sizeof(dest->magic));
	dest->len = be32_to_cpu(src->len);
	dest->type = be32_to_cpu(src->type);
	dest->checksum = be32_to_cpu(src->checksum);
	dest->offset = be32_to_cpu(src->offset);
}

static void
file_cbfs_fill_cache(u8 *start, u32 size, u32 align)
{
	CbfsCacheNode *cacheNode;
	CbfsCacheNode *newNode;
	CbfsFileHeader header;
	CbfsCacheNode **cacheTail = &fileCache;

	/* Clear out old information. */
	cacheNode = fileCache;
	while (cacheNode) {
		CbfsCacheNode *oldNode = cacheNode;
		cacheNode = cacheNode->next;
		free(oldNode->name);
		free(oldNode);
	}
	fileCache = NULL;

	while (size >= align) {
		CbfsFileHeader *fileHeader = (CbfsFileHeader *)start;
		u32 nameLen;
		u32 step;

		/* Check if there's a file here. */
		if (memcmp(goodFileMagic, &(fileHeader->magic),
				sizeof(fileHeader->magic))) {
			size -= align;
			start += align;
			continue;
		}

		newNode = (CbfsCacheNode *)malloc(sizeof(CbfsCacheNode));
		swap_file_header(&header, fileHeader);
		if (header.offset < sizeof(CbfsFileHeader) ||
				header.offset > header.len) {
			file_cbfs_result = CBFS_BAD_FILE;
			return;
		}
		newNode->next = NULL;
		newNode->type = header.type;
		newNode->data = start + header.offset;
		newNode->dataLength = header.len;
		nameLen = header.offset - sizeof(CbfsFileHeader);
		/* Add a byte for a NULL terminator. */
		newNode->name = (char *)malloc(nameLen + 1);
		strncpy(newNode->name,
			((char *)fileHeader) + sizeof(CbfsFileHeader),
			nameLen);
		newNode->name[nameLen] = 0;
		newNode->nameLength = nameLen;
		newNode->checksum = header.checksum;
		*cacheTail = newNode;
		cacheTail = &newNode->next;

		step = header.len;
		if (step % align)
			step = step + align - step % align;

		size -= step;
		start += step;
	}
	file_cbfs_result = CBFS_SUCCESS;
}

void
file_cbfs_init(uintptr_t endOfRom)
{
	CbfsHeader *headerInRom;
	u8 *startOfRom;
	initialized = 0;

	headerInRom = (CbfsHeader *)(uintptr_t)*(u32 *)(endOfRom - 3);
	swap_header(&header, headerInRom);

	if (header.magic != goodMagic || header.offset >
			header.romSize - header.bootBlockSize) {
		file_cbfs_result = CBFS_BAD_HEADER;
		return;
	}

	startOfRom = (u8 *)(endOfRom + 1 - header.romSize);

	file_cbfs_fill_cache(startOfRom + header.offset,
			     header.romSize, header.align);
	if (file_cbfs_result == CBFS_SUCCESS)
		initialized = 1;
}

const CbfsHeader *
file_cbfs_get_header(void)
{
	if (initialized) {
		file_cbfs_result = CBFS_SUCCESS;
		return &header;
	} else {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	}
}

CbfsFile
file_cbfs_get_first(void)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return fileCache;
	}
}

void
file_cbfs_get_next(CbfsFile *file)
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

CbfsFile
file_cbfs_find(const char *name)
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

const char *
file_cbfs_name(CbfsFile file)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return file->name;
	}
}

u32
file_cbfs_size(CbfsFile file)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return 0;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return file->dataLength;
	}
}

u32
file_cbfs_type(CbfsFile file)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return 0;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return file->type;
	}
}

long
file_cbfs_read(CbfsFile file, void *buffer, unsigned long maxsize)
{
	u32 size;

	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return -CBFS_NOT_INITIALIZED;
	}

	size = file->dataLength;
	if (maxsize && size > maxsize)
		size = maxsize;

	memcpy(buffer, file->data, size);

	file_cbfs_result = CBFS_SUCCESS;
	return size;
}
