/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define _LARGEFILE64_SOURCE

#include <linux/types.h>
#include <common.h>
#include <part.h>
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>

#include "ext4_utils.h"
#include "output_file.h"
#include "backed_block.h"

#define off_t   int

/* Write a contiguous region of data blocks from a memory buffer */
void write_data_block(block_dev_desc_t *dev_desc, disk_partition_t* info, u64 off, u8 *data, int len){
	int cnt;
	int lba;

	//printf("write_data_block> offset = %d len = %d \n", (u32)off, len);
	printf(".");

	lba = off >> 9;
	cnt = len >> 9;

	if(dev_desc->block_write(dev_desc->dev, (info->start)+lba , cnt, (ulong *)data) != cnt){
		printf("Write error ! \n");
	}
}

/* Write a contiguous region of data blocks from a file */
void write_data_file(u64 off, const char *file,
		     off_t offset, int len)
{
	error("do not entry here!");
	return;
}

