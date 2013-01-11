/*

Disk image builder
==================

This program builds a disk image from MBR and BBP images and writes GPT
and MBR entries accordingly

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
#include <Rpc.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include <structures.h>

char *mbr_img = null;
char *bbp_img = null;
uint32 bbp_size = 0; // 0MiB
char *efi_img = null;
uint32 efi_size = 0; // 0MiB
uint32 out_size = 1032192; // 1MiB
char *out_img = null;

int main(int argc, char *argv[]){
	int i = 0;
	char *arg;
	
	bool output_set = false;
	int32 tmp = 0;

	if (argc > 1){
		for (i = 1; i < argc; i ++){
			arg = argv[i];
			if ((*arg) == '-'){
				switch (*(++arg)){
					case 'h':
						print_help();
						return 0;
						break;
					case 'm':
						i++;
						mbr_img = argv[i];
						break;
					case 'b':
						i++;
						if (strlen(arg) > 1 && *(++arg) == 's'){
							tmp = (int32)atoi(argv[i]);
							if (tmp > 0){
								bbp_size = (uint32)tmp;
							}
						} else {
							bbp_img = argv[i];
						}
						break;
					case 'e':
						i++;
						if (strlen(arg) > 1 && *(++arg) == 's'){
							tmp = (int32)atoi(argv[i]);
							if (tmp > 0){
								efi_size = (uint32)tmp;
							}
						} else {
							efi_img = argv[i];
						}
						break;
					case 's':
						{
							i++;
							tmp = (int32)atoi(argv[i]);
							if (tmp > 0){
								out_size = (uint32)tmp;
							}
						}
						break;
				}
			} else {
				out_img = argv[i];
				output_set = true;
			}
		}
		if (output_set){
			build_img();
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
		"Usage: buildimg [OPTIONS] output.img\n\n"
		"Options:\n"
		"\t-b bbp.img\t\t- path to BBP image\n"
		"\t-bs length\t\t- BBP partition length in bytes (Default: size of bbp.img)*\n"
		"\t-e efi.img\t\t- path to EFI partition image\n"
		"\t-es length\t\t- EFI partition length in bytes (Default: size of efi.img)*\n"
		"\t-h \t\t\t- show help (this view)\n"
		"\t-m mbr.img\t\t- path to MBR image\n"
		"\t-s length\t\t- total image length in bytes (Default: 1032192)**\n\n"
		"\t\t* - always get's aligned to 1MiB boundary\n"
		"\t\t* - remember that some disks come with 63 sectors per track!\n"
	);
}

static uint64 fsize(FILE *fh){
	uint32 sz;
	fseek(fh, 0L, SEEK_END);
	sz = ftell(fh);
	fseek(fh, 0L, SEEK_SET);
	return sz;
}

static uint32 gpth_crc32(gpt_header_t *gpth){
	struct {
		char header[16];
		char rest[496];
	} gptbuff;
	memcpy(&gptbuff, gpth, sizeof(gpt_header_t));
	return crc32((char *)gptbuff.header, 16);
}
static uint32 gptp_crc32(gpt_part_entry_t *gptp, uint32 count){
	int size = count * sizeof(gpt_part_entry_t);
	char *gptbuff = (char *)calloc(size, sizeof(char));
	memcpy(gptbuff, gptp, size);
	uint32 crc = crc32(gptbuff, count * sizeof(gpt_part_entry_t));
	free(gptbuff);
	return crc;
}

void build_img(){
	uint32 i = 0;
	uint32 size = 0;
	uint32 size_aligned = 0;
	uint32 out_size_calc = 0;
	uint32 out_written = 0;
	uint32 lba_count = out_size / SECTOR;
	uint32 lba_offset = 33;
	uint32 part_count = 0;
	uint32 read_bytes = 0;
	uint32 write_bytes = 0;
	uint32 write_error = 0;
	int res = 0;
	char *buffer = (char *)calloc(SECTOR, sizeof(char));
	FILE *fimg = fopen(out_img, "wb");
	FILE *fmbr = 0, *fbbp = 0, *fefi = 0;
	if (fimg){
		// Get all the image sizes and prepare partition table
		gpt_part_entry_t *gptp = (gpt_part_entry_t *)calloc(128, sizeof(gpt_part_entry_t));
		if (mbr_img != null){
			fmbr = fopen(mbr_img, "rb");
			if (fmbr){
				size = fsize(fmbr);
				out_size_calc += SECTOR; // MBR
				out_size_calc += SECTOR; // GPT header
				out_size_calc += SECTOR * 32; // GPT partition array
				out_size_calc += SECTOR; // GPT header (secondary)
				out_size_calc += SECTOR * 32; // GPT partition array (secondary)
			} else {
				printf("Error: Failed to open MBR image file\n");
				return;
			}
		}
		if (bbp_img != null){
			fbbp = fopen(bbp_img, "rb");
			if (fbbp){
				size = fsize(fbbp);
				size_aligned = (uint32)ceil((float32)size / (float32)ONE_MB) * ONE_MB;
				out_size_calc += size_aligned;
				gptp[part_count].lba_start = lba_offset + 1;
				gptp[part_count].lba_end = lba_offset + (size_aligned / SECTOR) + 1;
				memcpy(gptp[part_count].name, L"BIOS Boot Partition", 19 * sizeof(wchar_t));
				// Standard BIOS Boot Partition GUID
				guid_t bbp_guid = {0x21686148, 0x6449, 0x6E6F, {0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49}};
				gptp[part_count].part_guid = bbp_guid;
				// Generate unique UUID
				UuidCreate((UUID *)&gptp[0].unique_guid);
				lba_offset = gptp[part_count].lba_end;
				part_count ++;
			} else {
				printf("Error: Failed to open BBP image file\n");
				return;
			}
		}
		if (efi_img != null){
			fefi = fopen(efi_img, "rb");
			if (fefi){
				size = fsize(fefi);
				size_aligned = (uint32)ceil((float32)size / (float32)ONE_MB) * ONE_MB;
				out_size_calc += size_aligned;
				gptp[part_count].lba_start = lba_offset + 1;
				gptp[part_count].lba_end = lba_offset + (size_aligned / SECTOR) + 1;
				memcpy(gptp[part_count].name, L"EFI System Partition", 20 * sizeof(wchar_t));
				// Standard EFI System Partition GUID
				guid_t efi_guid = {0xC12A7328, 0xF81F, 0x11D2, {0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};
				gptp[part_count].part_guid = efi_guid;
				// Generate unique UUID
				UuidCreate((UUID *)&gptp[0].unique_guid);
				lba_offset = gptp[part_count].lba_end;
				part_count ++;
			} else {
				printf("Error: Failed to open EFI partition image file\n");
				return;
			}
		}

		if (out_size_calc > out_size){
			printf("Warning: Image size will be %d bytes larger than you specified\n", (out_size_calc - out_size));
			out_size = out_size_calc;
			lba_count = out_size_calc / SECTOR;
		}

		// Prepare MBR record
		printf("Write Protectve MBR... ");
		lba_offset = 0;
		mbr_t *mbr = (mbr_t *)calloc(1, sizeof(mbr_t));
		if (fmbr){
			size = fsize(fmbr);
			if (size >= sizeof(mbr_t)){
				fread(mbr, sizeof(mbr_t), 1, fmbr);
			} else {
				// Just the bootcode then
				fread(mbr->bootcode, sizeof(char), min(440, size), fmbr);
				mbr->mbr_signature = 0xAA55;
			}
			fclose(fmbr);
		} else {
			mbr->mbr_signature = 0xAA55;
		}
		mbr->part[0].lba_start = 1;
		mbr->part[0].lba_size = lba_count - 1;
		mbr->part[0].type = 0xEE;
		mbr->part[0].status = 0x80;
		res = fwrite(mbr, sizeof(mbr_t), 1, fimg);
		free(mbr);
		lba_offset ++;
		out_written += sizeof(mbr_t);
		printf("%d bytes", sizeof(mbr_t));
		if (res){
			printf("OK\n");
		} else {
			printf("Error\n");
		}

		// Prepare GPT Primary header
		printf("Write primary GPT header... ");
		gpt_header_t *gpth = (gpt_header_t *)calloc(1, sizeof(gpt_header_t));
		memcpy(gpth->signature, "EFI PART", 8);
		gpth->version = 0x00000100;
		gpth->header_size = 92;
		gpth->header_crc32 = gpth_crc32(gpth);
		gpth->reserved = 0;
		gpth->gpt_lba_current = 1;
		gpth->gpt_lba_backup = lba_count;
		gpth->first_usable_lba = 34; // MBR + GPT header + 128 partition entries
		gpth->last_usable_lba = lba_count - 63; // total length - (MBR + (GPT header + 128 partition entries) * 2)
		// GUID: this-is-my-disk ok!
		guid_t disk_g = { 0x73696874, 0x7369, 0x796d, { 0x64, 0x69, 0x73, 0x6b, 0x20, 0x6f, 0x6b, 0x21 } };
		gpth->disk_guid = disk_g;
		gpth->part_arr_lba = 2;
		gpth->part_item_size = 128;
		gpth->part_item_count = 128;
		gpth->part_arr_crc32 = gptp_crc32(gptp, 128);
		res = fwrite(gpth, sizeof(gpt_header_t), 1, fimg);
		lba_offset ++;
		out_written += sizeof(gpt_header_t);
		printf("%d bytes ", sizeof(gpt_header_t));
		if (res){
			printf("OK\n");
		} else {
			printf("Error\n");
		}

		// Write GPT partition array (primary)
		printf("Write primary GPT partition array... ");
		res = fwrite(gptp, sizeof(gpt_part_entry_t), 128, fimg);
		lba_offset += 32;
		out_written += sizeof(gpt_part_entry_t) * 128;
		printf("%d bytes ", sizeof(gpt_part_entry_t) * 128);
		if (res){
			printf("OK\n");
		} else {
			printf("Error\n");
		}

		// Write BBP image
		if (fbbp){
			read_bytes = 0;
			write_bytes = 0;
			write_error = 0;
			printf("Write BBP image... ");
			size = fsize(fbbp);
			size_aligned = (uint32)ceil((float32)size / (float32)ONE_MB) * ONE_MB;
			while (!feof(fbbp)){
				res = fread(buffer, sizeof(char), SECTOR, fbbp);
				if (res){
					read_bytes += res;
					res = fwrite(buffer, sizeof(char), res, fimg);
					if (res){
						write_bytes += res;
					} else {
						write_error = 1;
					}
				}
			}
			printf("%d bytes read, %d bytes written of %d bytes total ", read_bytes, write_bytes, size);
			if (!write_error){
				printf("OK\n");
			} else {
				printf("Error\n");
			}
			printf("Add padding of %d bytes\n", (size_aligned - write_bytes));
			for (i = 0; i < size_aligned - size; i ++){
				fputc(0, fimg);
			}
			fclose(fbbp);
			lba_offset += size_aligned / SECTOR;
			out_written += size_aligned;
		}

		// Write EFI image
		if (fefi){
			read_bytes = 0;
			write_bytes = 0;
			write_error = 0;
			printf("Write EFI image... ");
			size = fsize(fefi);
			size_aligned = (uint32)ceil((float32)size / (float32)ONE_MB) * ONE_MB;
			while (!feof(fbbp)){
				res = fread(buffer, sizeof(char), SECTOR, fefi);
				if (res){
					read_bytes += res;
					res = fwrite(buffer, sizeof(char), res, fimg);
					if (res){
						write_bytes += res;
					} else {
						write_error = 1;
					}
				}
			}
			printf("%d bytes read, %d bytes written of %d bytes total ", read_bytes, write_bytes, size);
			if (!write_error){
				printf("OK\n");
			} else {
				printf("Error\n");
			}
			printf("Add padding of %d bytes\n", (size_aligned - write_bytes));
			for (i = 0; i < size_aligned - size; i ++){
				fputc(0, fimg);
			}
			fclose(fefi);
			lba_offset += size_aligned / SECTOR;
			out_written += size_aligned;
		}

		// Add extra padding after all the partitions, before secondary GPT

		printf("Add empty space padding of %d bytes\n", (out_size - (33 * SECTOR) - out_written));
		for (i = 0; i < (out_size - (33 * SECTOR) - out_written); i ++){
			fputc(0, fimg);
		}
		

		// Write GPT partition array (primary)
		printf("Write secondary GPT partition array... ");
		res = fwrite(gptp, sizeof(gpt_part_entry_t), 128, fimg);
		printf("%d bytes ", sizeof(gpt_part_entry_t) * 128);
		if (res){
			printf("OK\n");
		} else {
			printf("Error\n");
		}
		
		// Prepare GPT Secondary header
		gpth->part_arr_lba = lba_offset;
		// Write Secondary GPT header
		printf("Write secondary GPT header... ");
		res = fwrite(gpth, sizeof(gpt_header_t), 1, fimg);
		printf("%d bytes ", sizeof(gpt_header_t));
		if (res){
			printf("OK\n");
		} else {
			printf("Error\n");
		}
		fclose(fimg);
		printf("DONE, thank you...\n\n");
	} else {
		printf("Error: could not open the output file\n");
	}
}
