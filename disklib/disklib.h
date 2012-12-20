/*

DiskLib
=======

This is a redistributable header file for disklib.

License (BSD-3)
===============

Copyright (c) 2012, Gusts 'gusC' Kaksis <gusts.kaksis@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __diskedit_h
#define __diskedit_h

#include "common.h"

/**
* MBR partition entry information structure
*/
typedef struct {
	bool bootable;
	uint8 type;
	uint32 lba_start;
	uint32 lba_end;
	uint32 lba_size;
} mbr_part_info_t;
/**
* MBR information structure
*/
typedef struct {
	bool gpt_protect;
	uint8 part_count;
	mbr_part_info_t *part;
} mbr_info_t;
/**
* GPT partition entry information structure
*/
typedef struct {
	bool bios_bootable;
	guid_t part_guid;
	guid_t unique_guid;
	uint64 lba_start;
	uint64 lba_end;
	uint64 lba_size;
	char name[36];
} gpt_part_info_t;
/**
* GPT information structure
*/
typedef struct {
	guid_t disk_guid;
	uint64 usable_lba_start;
	uint64 usable_lba_end;
	uint32 part_count;
	gpt_part_info_t *part;
} gpt_info_t;
/**
* Disk geometry information structure
*/
typedef struct {
	uint64 tracks;
	uint32 heads;
	uint32 spt;
	uint32 bps;
} disk_chs_t;
/**
* Disk information structure
*/
typedef struct {
	disk_chs_t chs;
	uint64 size;
	bool fixed;
	mbr_info_t mbr;
	gpt_info_t gpt;
} disk_info_t;

// disk.cpp
/**
* Get the total number of available physical devices
* @return uint32
*/
uint32 disk_count();
/**
* Open a physical disk
* @param uint32 - disk index (zero based)
* @return handle_t - disk handle
*/
handle_t disk_open(uint32 diskidx);
/**
* Close a physical disk handle
* @param handle_t - disk handle
* @return void
*/
void disk_close(handle_t h);
/**
* Get disk info
* @param handle_t - disk handle
* @param[out] disk_info_t * - a pointer to disk info structure
* @return bool - true on success or false on fail
*/
bool disk_info(handle_t h, disk_info_t *info);
/**
* Read from the disk
* @param handle_t - disk handle
* @param[out] char * - pointer to data buffer
* @param uint32 - how many bytes to read
* @param uint64 - read offset
* @return uint32 - how many bytes have been read
*/
uint32 disk_read(handle_t h, char *buffer, uint32 len, uint64 offset);
/**
* Write to disk
* @param handle_t - disk handle
* @param[in] char * - pointer to data buffer
* @param uint32 - how many bytes to write
* @param uint64 - write offset
* @return uint32 - how many bytes have been written
*/
uint32 disk_write(handle_t h, char *buffer, uint32 len, uint64 offset);

// image.cpp
/**
* Open a disk image
* @param const char * - file path
* @return handle_t - image handle
*/
handle_t image_open(const char *filepath);
/**
* Close a disk image handle
* @param handle_t - image handle
* @return void
*/
void image_close(handle_t h);
/**
* Try to read disk info from an image file
* @param handle_t - image handle
* @param[out] disk_info_t * - a pointer to disk info structure
* @return bool - true on success or false on fail
*/
bool image_info(handle_t h, disk_info_t *info);
/**
* Read from the image
* @param handle_t - image handle
* @param[out] char * - pointer to data buffer
* @param uint32 - how many bytes to read
* @param uint64 - read offset
* @return uint32 - how many bytes have been read
*/
uint32 image_read(handle_t h, char *buffer, uint32 len, uint64 offset);
/**
* Write to the image
* @param handle_t - image handle
* @param[in] char * - pointer to data buffer
* @param uint32 - how many bytes to write
* @param uint64 - write offset
* @return uint32 - how many bytes have been written
*/
uint32 image_write(handle_t h, char *buffer, uint32 len, uint64 offset, bool rewrite);

// utils.cpp
/**
* Helper function - convert bytes into pretty-string
* @param[out] char * - pretty-string buffer
* @param uint64 - size in bytes
* @return void
*/
void format_size(char *out, uint64 size);

#endif
