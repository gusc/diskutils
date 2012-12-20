/*

Disk operation functions
========================

This file contains functions for physical disk operations (See: disklib.h)

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

#include "disklib.h"
#include "structures.h"
#include <Windows.h>
#include <stdio.h>

uint32 disk_count(){
	uint32 i = 0;
	uint32 max_disk = 16;
	handle_t h = null;
	for (i = 0; i < max_disk; i ++){
		h = disk_open(i);
		if (h == null){
			max_disk = i;
		}
		disk_close(h);
	}
	return max_disk;
}

handle_t disk_open(uint32 diskidx){
	char *fmt = "\\\\.\\PhysicalDrive%d";
	char path[32] = "";
	handle_t h = null;
	sprintf(path, fmt, diskidx);
	h = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (h != INVALID_HANDLE_VALUE){
		return h;
	} else {
		CloseHandle(h);
		h = null;
	}
	return h;
}

void disk_close(handle_t h){
	if (h != null){
		CloseHandle(h);
		h = null;
	}
}


bool disk_info(handle_t h, disk_info_t *info){
	bool res = false;
	ZeroMemory(info, sizeof(disk_info_t));
	if (h != null){
		uint32 i = 0, j = 0, k = 0;
		
		DWORD dwResult;
		DISK_GEOMETRY driveInfo;
		
		mbr_t mbr;
		gpt_header_t gpt_header;
		gpt_part_entry_t *gpt_part;
		uint32 mbr_part_count = 4;
		uint32 part_per_sect = 4;
		uint32 part_sect = 1;
		uint32 read_offset = 0;

		if (DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof (driveInfo), &dwResult, NULL)){
			info->chs.tracks = driveInfo.Cylinders.QuadPart;
			info->chs.heads = driveInfo.TracksPerCylinder;
			info->chs.spt = driveInfo.SectorsPerTrack;
			info->chs.bps = driveInfo.BytesPerSector;
			info->fixed = (driveInfo.MediaType == FixedMedia);
			info->size = info->chs.tracks * info->chs.heads * info->chs.spt * info->chs.bps;
			
			dwResult = SetFilePointer(h, read_offset, NULL, FILE_BEGIN);
			if (dwResult != INVALID_SET_FILE_POINTER){
				if (ReadFile(h, &mbr, sizeof(mbr_t), &dwResult, NULL)){
					read_offset += info->chs.bps;
					if (mbr.mbr_signature == 0xAA55){
						info->mbr.part_count = 0;
						for (i = 0; i < mbr_part_count; i ++){
							if (mbr.part[i].type == 0xEE){
								// GPT protective
								info->mbr.gpt_protect = true;
							}
							if (mbr.part[i].type != 0x00){
								// Used partition
								info->mbr.part_count ++;
							}
						}
						info->mbr.part = (mbr_part_info_t *)calloc(info->mbr.part_count, sizeof(mbr_part_info_t));
						j = 0;
						for (i = 0; i < mbr_part_count; i ++){
							if (mbr.part[i].type != 0x00){
								// Only gather information about used partitions
								info->mbr.part[j].type = mbr.part[i].type;
								if ((mbr.part[i].status & 0x80) == 0x80){
									// Bootable partition
									info->mbr.part[j].bootable = true;
								}
								info->mbr.part[j].lba_start = mbr.part[i].lba_start;
								info->mbr.part[j].lba_end = mbr.part[i].lba_start + mbr.part[i].lba_size;
								info->mbr.part[j].lba_size = mbr.part[i].lba_size;
								j ++;
							}
						}
					}
					if (info->mbr.gpt_protect){
						dwResult = SetFilePointer(h, read_offset, NULL, FILE_BEGIN);
						if (dwResult != INVALID_SET_FILE_POINTER){
							if (ReadFile(h, &gpt_header, sizeof(gpt_header_t), &dwResult, NULL)){
								read_offset += info->chs.bps;
								
								memcpy(&info->gpt.disk_guid, &gpt_header.disk_guid, sizeof(guid_t));
								info->gpt.usable_lba_start = gpt_header.first_usable_lba;
								info->gpt.usable_lba_end = gpt_header.last_usable_lba;
								
								part_per_sect = info->chs.bps / gpt_header.part_item_size;
								part_sect = gpt_header.part_item_count / part_per_sect;
								gpt_part = (gpt_part_entry_t *)calloc(part_per_sect, sizeof(gpt_part_entry_t));

								// Test real partition count
								info->gpt.part_count = 0;
								for (i = 0; i < part_sect; i ++){
									dwResult = SetFilePointer(h, read_offset + (info->chs.bps * i), NULL, FILE_BEGIN);
									if (dwResult != INVALID_SET_FILE_POINTER){
										if (ReadFile(h, gpt_part, sizeof(gpt_part_entry_t) * part_per_sect, &dwResult, NULL)){
											for (j = 0; j < part_per_sect; j ++){
												if (gpt_part[j].lba_start >= info->gpt.usable_lba_start && gpt_part[j].lba_end <= info->gpt.usable_lba_end && gpt_part[j].lba_end >= gpt_part[j].lba_start){
													info->gpt.part_count ++;
												}
											}
										}
									}
								}
								info->gpt.part = (gpt_part_info_t *)calloc(info->gpt.part_count, sizeof(gpt_part_info_t));

								// Read real partitions
								k = 0;
								for (i = 0; i < part_sect; i ++){
									dwResult = SetFilePointer(h, read_offset + (info->chs.bps * i), NULL, FILE_BEGIN);
									if (dwResult != INVALID_SET_FILE_POINTER){
										if (ReadFile(h, gpt_part, sizeof(gpt_part_entry_t) * part_per_sect, &dwResult, NULL)){
											for (j = 0; j < part_per_sect; j ++){
												if (gpt_part[j].lba_start >= info->gpt.usable_lba_start && gpt_part[j].lba_end <= info->gpt.usable_lba_end && gpt_part[j].lba_end >= gpt_part[j].lba_start){
													info->gpt.part[k].bios_bootable = ((gpt_part[j].attributes & 0x2) == 0x2);
													info->gpt.part[k].lba_start = gpt_part[j].lba_start;
													info->gpt.part[k].lba_end = gpt_part[j].lba_end;
													info->gpt.part[k].lba_size = gpt_part[j].lba_end - gpt_part[j].lba_start + 1; // end is inclusive
													memcpy(&info->gpt.part[k].part_guid, &gpt_part[j].part_guid, sizeof(guid_t));
													memcpy(&info->gpt.part[k].unique_guid, &gpt_part[j].unique_guid, sizeof(guid_t));
													wcstombs(info->gpt.part[k].name, gpt_part[j].name, 36);
													k ++;
												}
											}
										}
									}
								}
								res = true;
							}
						}
					} else {
						res = true;
					}
				}
			}
		}
	}
	return res;
}

uint32 disk_read(handle_t h, char *buffer, uint32 len, uint64 offset){
	// TODO: implement offset alignament to sector size
	DWORD dwResult;
	pack64 offpack;
	offpack.qword = offset;
	dwResult = SetFilePointer(h, offpack.dword.low, (PLONG)&offpack.dword.high, FILE_BEGIN);
	if (dwResult != INVALID_SET_FILE_POINTER){
		if (ReadFile(h, buffer, sizeof(char) * len, &dwResult, NULL)){
			return (uint32)dwResult;
		}
	}
	return 0;
}
uint32 disk_write(handle_t h, char *buffer, uint32 len, uint64 offset){
	// TODO: implement offset alignament to sector size
	DWORD dwResult;
	pack64 offpack;
	offpack.qword = offset;
	dwResult = SetFilePointer(h, offpack.dword.low, (PLONG)&offpack.dword.high, FILE_BEGIN);
	if (dwResult != INVALID_SET_FILE_POINTER){
		if (WriteFile(h, buffer, sizeof(char) * len, &dwResult, NULL)){
			return (uint32)dwResult;
		}
	}
	return 0;
}