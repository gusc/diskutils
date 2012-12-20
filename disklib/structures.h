/*

Memory mapped structure definitions
===================================

This file contains memory mapped structures, that are used to read or write 
data to and from disks or images in a structured fassion.

Alignament should be disabled when including these structures.

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

#ifndef __structures_h
#define __structures_h

#pragma pack(1)

#include "common.h"

/**
* MBR partition entry
* \see http://en.wikipedia.org/wiki/Master_boot_record#PTE
*/
typedef struct {
	uint8 status;
	uint8 chs_start[3];
	uint8 type;
	uint8 chs_end[3];
	uint32 lba_start;
	uint32 lba_size;
} mbr_part_t; // 16 bytes

/**
* MBR sector
* \see http://en.wikipedia.org/wiki/Master_boot_record#Sector_layout
*/
typedef struct {
	uchar bootcode[440];
	uint32 disk_signature;
	uint16 empty;
	mbr_part_t part[4];
	uint16 mbr_signature;
} mbr_t; // 512 bytes

/**
* GPT partition entry
* \see http://en.wikipedia.org/wiki/GUID_Partition_Table#Partition_entries
*/
typedef struct {
	guid_t part_guid; 
	guid_t unique_guid;
	uint64 lba_start;
	uint64 lba_end;
	uint64 attributes;
	wchar_t name[36];
} gpt_part_entry_t; // 128 bytes

/**
* GPT header
* \see http://en.wikipedia.org/wiki/GUID_Partition_Table#Partition_table_header_.28LBA_1.29
*/
typedef struct {
	char signature[8];
	uint32 version;
	uint32 header_size;
	uint32 header_crc32;
	uint32 reserved;
	uint64 gpt_lba_current;
	uint64 gpt_lba_backup;
	uint64 first_usable_lba;
	uint64 last_usable_lba;
	guid_t disk_guid;
	uint64 part_arr_lba;
	uint32 part_item_count;
	uint32 part_item_size;
	uint32 part_arr_crc2;
	char reserved2[420];
} gpt_header_t; // 512 bytes

#endif
