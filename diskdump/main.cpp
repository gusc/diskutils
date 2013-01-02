/*

Diskdump
========

This program allows the user to read or write physical disks or image files
using command line.

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

#include <Windows.h>
#include <stdio.h>
#include <math.h>

#include "main.h"

char *input = null;
char *output = null;
uint64 input_off = 0;
uint64 output_off = 0;
uint32 size = 512;

int main(int argc, char *argv[]){
	int i = 0;
	char *arg;
	
	bool input_set = false;
	bool output_set = false;

	if (argc > 1){
		for (i = 1; i < argc; i ++){
			arg = argv[i];
			if ((*arg) == '-'){
				switch (*(++arg)){
					case 'h':
						print_help();
						return 0;
						break;
					case 'i':
						i++;
						if (strlen(arg) > 1 && *(++arg) == 'o'){
							input_off = atoi(argv[i]);
						} else {
							input = argv[i];
							input_set = true;
						}
						break;
					case 'o':
						i++;
						if (strlen(arg) > 1 && *(++arg) == 'o'){
							output_off = atoi(argv[i]);
						} else {
							output = argv[i];
							output_set = true;
						}
						break;
					case 's':
						i++;
						size = atoi(argv[i]);
						break;
					case 'l':
						print_disk_list();
						return 0;
						break;
					case 'd':
						i++;
						print_disk_info(argv[i]);
						return 0;
						break;
				}
			}
		}
		if (input_set && output_set){
			copy_data();
		} else {
			printf("Invalid arguments\n");
			print_help();
		}
	} else {
		printf("No arguments\n");
		print_help();
	}

	return 0;
}

void print_help(){
	printf(
		"Usage: diskedit [OPTIONS]\n\n"
		"Options:\n"
		"\t-d diskidx\t\t\t- show disk info\n"
		"\t-h \t\t\t\t- show help (this view)\n"
		"\t-i path|diskidx|diskidx:partidx\t- input stream\n"
		"\t-io offset\t\t\t- transfer start offset for input stream (Default: 0)\n"
		"\t-l \t\t\t\t- list all available physical drives and partitions\n"		
		"\t-o path|diskidx|diskidx:partidx\t- output stream\n"
		"\t-oo offset\t\t\t- transfer start offset for output stream (Default: 0)\n"
		"\t-s length\t\t\t- transfer size in bytes (Default: 512)\n"
	);
}

void print_disk_list(){
	uint32 d = 0, p = 0, i = 0, x = 0;
	uint32 dcount = disk_count();
	uint32 pcount = 0;
	handle_t disk = null;
	disk_info_t info;
	char *disk_tmp = " %d\t\t\t\\\\.\\PhysicalDrive%d\t\t%s\n";
	char *part_tmp = " %d:%d\t\t\t\\Device\\Harddisk%d\\Partition%d\t%s\n";
	char size[64] = "";

	printf(
		" diskidx[:partidx]\tDisk path\t\t\tSize\n"
		"-----------------------------------------------------------------\n"
	);
	for (d = 0; d < dcount; d ++){
		disk = disk_open(d);
		if (disk != null){
			disk_info(disk, &info);
			format_size(size, info.size);
			printf(disk_tmp, d, d, size);
			if (info.mbr.gpt_protect){
				for (i = 0; i < info.gpt.part_count; i ++){
					format_size(size, info.gpt.part[i].lba_size * ((uint64)info.chs.bps));
					printf(part_tmp, d, p, d, p, size);
				}
			} else {
				for (i = 0; i < info.mbr.part_count; i ++){
					format_size(size, ((uint64)info.mbr.part[i].lba_size * (uint64)info.chs.bps));
					printf(part_tmp, d, p, d, p, size);
				}
			}
		}
	}
}
void print_disk_info(char *selector){
	uint32 i = 0, j = 0;
	uint32 diskidx = atoi(selector);
	disk_info_t info;
	char size[64] = "";
	//char *name;
	handle_t disk = disk_open(diskidx);

	if (disk != null){
		disk_info(disk, &info);
		printf(
			" Info for \\\\.\\PhysicalDrive%d\n"
			"----------------------------------\n",
			disk
		);
		format_size(size, info.size);

		printf("Size:\t\t\t%s\n", size);
		printf("Type:\t\t\t%s\n\n", info.fixed ? "Fixed" : "Removable");
		printf("Cylinders:\t\t%I64u\n", info.chs.tracks);
		printf("Heads:\t\t\t%u\n", info.chs.heads);
		printf("Sectors per Cylinder:\t%u\n", info.chs.spt);
		printf("Sectors size:\t\t%u\n\n", info.chs.bps);
		if (info.mbr.gpt_protect){
			printf(" MBR Partitions: %u (GPT Protective)\n", info.mbr.part_count);
			printf(" GPT Partitions: %u\n", info.gpt.part_count);
			printf(
				"-------------------------------------------------------------------------\n"
				" partidx : Type GUID\n"
				" boot\tstart\t\t\tend\t\t\tsize\n"
				"-------------------------------------------------------------------------\n"
			);
			for (i = 0; i < info.gpt.part_count; i ++){
				format_size(size, info.gpt.part[i].lba_size * ((uint64)info.chs.bps));
				printf(" %d : ", i);
				//printf(info.gpt.part[i].name);
				//printf("\n");
				printf(" %08X-%04X-%04X-%02X%02X-",
					info.gpt.part[i].part_guid.data1,
					info.gpt.part[i].part_guid.data2,
					info.gpt.part[i].part_guid.data3,
					info.gpt.part[i].part_guid.data4[0],
					info.gpt.part[i].part_guid.data4[1]);
				for (j = 2; j < 8; j++){
					printf("%02X", info.gpt.part[i].part_guid.data4[j]);
				}
				printf("\n %s\t%16I64u\t%16I64u\t%s\n", 
					info.gpt.part[i].bios_bootable ? "Yes" : "No", 
					info.gpt.part[i].lba_start, info.gpt.part[i].lba_end, size); 
			}
		} else {
			printf(" MBR Partitions: %u\n", info.mbr.part_count);
			printf(
				"------------------------------------------------------------------------\n"
				" partidx\tboot\ttype\tstart\t\tend\t\tsize\n"
				"------------------------------------------------------------------------\n"
			);
			for (i = 0; i < info.mbr.part_count; i ++){
				format_size(size, ((uint64)info.mbr.part[i].lba_size) * ((uint64)info.chs.bps));
				printf(" %d\t\t%s\t%02X\t%10u\t%10u\t%s\n", 
					i, info.mbr.part[i].bootable ? "Yes" : "No", info.mbr.part[i].type, 
					info.mbr.part[i].lba_start, info.mbr.part[i].lba_end, size); 
			}
		}
	}
}

void copy_data(){
	char *fmt_disk = "%d";
	char *fmt_part = "%d:%d";
	
	
	uint32 diskidx = 0;
	
	eType in_type = kTypeNone;
	uint32 in_partidx = 0;
	handle_t in = null;
	disk_info_t in_info;
	
	eType out_type = kTypeNone;
	uint32 out_partidx = 0;
	handle_t out = null;
	disk_info_t out_info;

	uint32 buffer_size = 512;
	char *buffer;
	uint32 i = 0;
	uint32 takes = 0;

	if (sscanf(input, fmt_part, &diskidx, &in_partidx) == 2){
		in = disk_open(diskidx);
		if (in != null){
			disk_info(in, &in_info);
			buffer_size = in_info.chs.bps;
			in_type = kTypePart;
			printf("Read from: \\Device\\Harddisk%d\\Partition%d\n", diskidx, in_partidx);
		} else {
			printf("Error opening: \\Device\\Harddisk%d\\Partition%d\n", diskidx, in_partidx);
			return;
		}
	} else if (sscanf(input, fmt_disk, &diskidx) == 1){
		in = disk_open(diskidx);
		if (in != null){
			disk_info(in, &in_info);
			buffer_size = in_info.chs.bps;
			in_type = kTypeDisk;
			printf("Read from: \\\\.\\PhysicalDrive%d\n", diskidx);
		} else {
			printf("Error opening: \\\\.\\PhysicalDrive%d\n", diskidx);
			return;
		}
	} else {
		in = image_open(input);
		if (in != null){
			in_type = kTypeImage;
			printf("Read from: %s\n", input);
		} else {
			printf("Error opening: %s\n", input);
			return;
		}
	}
	if (sscanf(output, fmt_part, &diskidx, &out_partidx) == 2){
		out = disk_open(diskidx);
		if (out != null){
			disk_info(out, &out_info);
			buffer_size = max(buffer_size, out_info.chs.bps);
			out_type = kTypePart;
			printf("Write to: \\Device\\Harddisk%d\\Partition%d\n", diskidx, out_partidx);
		} else {
			printf("Error opening: \\Device\\Harddisk%d\\Partition%d\n", diskidx, out_partidx);
			return;
		}
	} else if (sscanf(output, fmt_disk, &diskidx) == 1){
		out = disk_open(diskidx);
		if (out != null){
			disk_info(out, &out_info);
			buffer_size = max(buffer_size, out_info.chs.bps);
			out_type = kTypeDisk;
			printf("Write to: \\\\.\\PhysicalDrive%d\n", diskidx);
		} else {
			printf("Error opening: \\\\.\\PhysicalDrive%d\n", diskidx);
			return;
		}
	} else {
		out = image_open(output);
		if (out != null){
			out_type = kTypeImage;
			printf("Write to: %s\n", output);
		} else {
			printf("Error opening: %s\n", output);
			return;
		}
	}

	printf("\n");
	printf("Read %d bytes from offset %d\n", size, input_off);
	printf("Write at offset %d\n", output_off);
	printf("Buffer size %d bytes\n", buffer_size);

	buffer = (char *)calloc(buffer_size, sizeof(char));
	takes = (uint32)ceil((float32)size / (float32)buffer_size);

	for (i = 0; i < takes; i ++){
		switch (in_type){
			case kTypeImage:
				image_read(in, buffer, buffer_size, input_off + ((uint64)i * (uint64)buffer_size));
				break;
			case kTypePart:
				if (in_info.mbr.gpt_protect){
					disk_read(in, buffer, buffer_size, in_info.gpt.part[in_partidx].lba_start + input_off + ((uint64)i * (uint64)buffer_size));
				} else {
					disk_read(in, buffer, buffer_size, ((uint64)in_info.mbr.part[in_partidx].lba_start) + input_off + ((uint64)i * (uint64)buffer_size));
				}
				break;
			case kTypeDisk:
				disk_read(in, buffer, buffer_size, input_off + ((uint64)i * buffer_size));
				break;
		}
		switch (out_type){
			case kTypeImage:
				image_write(out, buffer, buffer_size, output_off + ((uint64)i * buffer_size), false);
				break;
			case kTypePart:
				if (out_info.mbr.gpt_protect){
					disk_write(in, buffer, buffer_size, out_info.gpt.part[out_partidx].lba_start + output_off + ((uint64)i * (uint64)buffer_size));
				} else {
					disk_write(in, buffer, buffer_size, ((uint64)out_info.mbr.part[out_partidx].lba_start) + output_off + ((uint64)i * (uint64)buffer_size));
				}
				break;
			case kTypeDisk:
				disk_write(out, buffer, buffer_size, output_off + ((uint64)i * 512));
				break;
		}
	}

	switch (in_type){
		case kTypeImage:
			image_close(in);
			break;
		case kTypePart:
		case kTypeDisk:
			disk_close(in);
			break;
	}
	switch (out_type){
		case kTypeImage:
			image_close(out);
			break;
		case kTypePart:
		case kTypeDisk:
			disk_close(out);
			break;
	}
}
