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
#include <linux/types.h>
#include <common.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <command.h>

#include <environment.h>


#include "ext4_utils.h"
#include "ext4.h"
#include "output_file.h"
#include "backed_block.h"
#include "uuid.h"
#include "allocate.h"
#include "indirect.h"
#include "extent.h"

#include "ext4.h"
#include "jbd2.h"

struct fs_info info;
struct fs_aux_info aux_info;

static unsigned int alloc_ptr;

void ext4_malloc_init(void)
{
	alloc_ptr = CFG_LOAD_ADDR;
}

void* ext4_malloc(unsigned int size)
{
	unsigned int old;

	if(alloc_ptr + size > CFG_MEMTEST_END)
		return 0;

	old = alloc_ptr;

	alloc_ptr += size;

	return (void*)old;
}

void* ext4_calloc(unsigned int size, unsigned int cnt)
{
	unsigned int old;

	if(alloc_ptr + size*cnt > CFG_MEMTEST_END)
		return 0;

	old = alloc_ptr;

	alloc_ptr += size*cnt;

	memset((void*)old, 0, size*cnt);

	return (void*)old;
}

void ext4_free(void* ptr)
{
	return;
}

/* returns 1 if a is a power of b */
static int is_power_of(int a, int b)
{
	while (a > b) {
		if (a % b)
			return 0;
		a /= b;
	}

	return (a == b) ? 1 : 0;
}

/* Returns 1 if the bg contains a backup superblock.  On filesystems with
   the sparse_super feature, only block groups 0, 1, and powers of 3, 5,
   and 7 have backup superblocks.  Otherwise, all block groups have backup
   superblocks */
int ext4_bg_has_super_block(int bg)
{
	/* Without sparse_super, every block group has a superblock */
	if (!(info.feat_ro_compat & EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER))
		return 1;

	if (bg == 0 || bg == 1)
		return 1;

	if (is_power_of(bg, 3) || is_power_of(bg, 5) || is_power_of(bg, 7))
		return 1;

	return 0;
}

void write_ext4_image(block_dev_desc_t *dev_desc, disk_partition_t* part, const char *filename, int gz, int sparse)
{
	/* The write_data* functions expect only block aligned calls.
	 * This is not an issue, except when we write out the super
	 * block on a system with a block size > 1K.  So, we need to
	 * deal with that here.
	 */

	if (info.block_size > 1024) {
		u8 buf[4096] = { 0 }; 	/* The larget supported ext4 block size */
		memcpy(buf + 1024, (u8*)aux_info.sb, 1024);
		write_data_block(dev_desc, part, (u64)0, buf, info.block_size);

	} else {
		write_data_block(dev_desc, part, (u64)1024, (u8*)aux_info.sb, 1024);
	}

	write_data_block(dev_desc, part, (u64)(aux_info.first_data_block + 1) * info.block_size,
			 (u8*)aux_info.bg_desc,
			 aux_info.bg_desc_blocks * info.block_size);

	for_each_data_block(dev_desc,part,write_data_block, write_data_file);

	printf("\nWrite Done.\n");
}

/* Compute the rest of the parameters of the filesystem from the basic info */
void ext4_create_fs_aux_info()
{
	aux_info.first_data_block = (info.block_size > 1024) ? 0 : 1;
	aux_info.len_blocks = (u32)info.len / info.block_size;

	aux_info.inode_table_blocks = EXT4_DIV_ROUND_UP(info.inodes_per_group * info.inode_size,
		info.block_size);
	aux_info.groups = EXT4_DIV_ROUND_UP((u32)aux_info.len_blocks - aux_info.first_data_block,
		info.blocks_per_group);
	aux_info.blocks_per_ind = info.block_size / sizeof(u32);
	aux_info.blocks_per_dind = aux_info.blocks_per_ind * aux_info.blocks_per_ind;
	aux_info.blocks_per_tind = aux_info.blocks_per_dind * aux_info.blocks_per_dind;

	aux_info.bg_desc_blocks =
		EXT4_DIV_ROUND_UP(aux_info.groups * sizeof(struct ext2_group_desc),
			info.block_size);

	aux_info.bg_desc_reserve_blocks =
		EXT4_DIV_ROUND_UP(aux_info.groups * 1024 * sizeof(struct ext2_group_desc),
			info.block_size) - aux_info.bg_desc_blocks;

	if (aux_info.bg_desc_reserve_blocks > aux_info.blocks_per_ind)
		aux_info.bg_desc_reserve_blocks = aux_info.blocks_per_ind;

	aux_info.default_i_flags = EXT4_NOATIME_FL;

	u32 last_group_size = (u32)aux_info.len_blocks % info.blocks_per_group;
	u32 last_header_size = 2 + aux_info.inode_table_blocks;
	if (ext4_bg_has_super_block(aux_info.groups - 1))
		last_header_size += 1 + aux_info.bg_desc_blocks +
			aux_info.bg_desc_reserve_blocks;
	if (last_group_size > 0 && last_group_size < last_header_size) {
		aux_info.groups--;
		aux_info.len_blocks -= last_group_size;
	}

	aux_info.sb = ext4_calloc(info.block_size, 1);
	if (!aux_info.sb)
		critical_error_errno("ext4_calloc");

	aux_info.bg_desc = ext4_calloc(info.block_size, aux_info.bg_desc_blocks);
	if (!aux_info.bg_desc)
		critical_error_errno("ext4_calloc");
}

void ext4_free_fs_aux_info()
{
	ext4_free(aux_info.sb);
	ext4_free(aux_info.bg_desc);
}

/* Fill in the superblock memory buffer based on the filesystem parameters */
void ext4_fill_in_sb()
{
	unsigned int i;
	struct ext4_super_block *sb = aux_info.sb;

	sb->s_inodes_count = info.inodes_per_group * aux_info.groups;
	sb->s_blocks_count_lo = aux_info.len_blocks;
	sb->s_r_blocks_count_lo = 0;
	sb->s_free_blocks_count_lo = 0;
	sb->s_free_inodes_count = 0;
	sb->s_first_data_block = aux_info.first_data_block;
	sb->s_log_block_size = log_2(info.block_size / 1024);
	sb->s_obso_log_frag_size = log_2(info.block_size / 1024);
	sb->s_blocks_per_group = info.blocks_per_group;
	sb->s_obso_frags_per_group = info.blocks_per_group;
	sb->s_inodes_per_group = info.inodes_per_group;
	sb->s_mtime = 0;
	sb->s_wtime = 0;
	sb->s_mnt_count = 0;
	sb->s_max_mnt_count = 0xFFFF;
	sb->s_magic = EXT4_SUPER_MAGIC;
	sb->s_state = EXT4_VALID_FS;
	sb->s_errors = EXT4_ERRORS_RO;
	sb->s_minor_rev_level = 0;
	sb->s_lastcheck = 0;
	sb->s_checkinterval = 0;
	sb->s_creator_os = EXT4_OS_LINUX;
	sb->s_rev_level = EXT4_DYNAMIC_REV;
	sb->s_def_resuid = EXT4_DEF_RESUID;
	sb->s_def_resgid = EXT4_DEF_RESGID;

	sb->s_first_ino = EXT4_GOOD_OLD_FIRST_INO;
	sb->s_inode_size = info.inode_size;
	sb->s_block_group_nr = 0;
	sb->s_feature_compat = info.feat_compat;
	sb->s_feature_incompat = info.feat_incompat;
	sb->s_feature_ro_compat = info.feat_ro_compat;
	generate_uuid("uboot_ext4fs", info.label, sb->s_uuid);
	memset(sb->s_volume_name, 0, sizeof(sb->s_volume_name));
	strncpy(sb->s_volume_name, info.label, sizeof(sb->s_volume_name));
	memset(sb->s_last_mounted, 0, sizeof(sb->s_last_mounted));
	sb->s_algorithm_usage_bitmap = 0;

	sb->s_reserved_gdt_blocks = aux_info.bg_desc_reserve_blocks;
	sb->s_prealloc_blocks = 0;
	sb->s_prealloc_dir_blocks = 0;

	//memcpy(sb->s_journal_uuid, sb->s_uuid, sizeof(sb->s_journal_uuid));
	if (info.feat_compat & EXT4_FEATURE_COMPAT_HAS_JOURNAL)
		sb->s_journal_inum = EXT4_JOURNAL_INO;
	sb->s_journal_dev = 0;
	sb->s_last_orphan = 0;
	sb->s_hash_seed[0] = 0; /* FIXME */
	sb->s_def_hash_version = DX_HASH_TEA;
	sb->s_reserved_char_pad = EXT4_JNL_BACKUP_BLOCKS;
	sb->s_desc_size = sizeof(struct ext2_group_desc);
	sb->s_default_mount_opts = 0; /* FIXME */
	sb->s_first_meta_bg = 0;
	sb->s_mkfs_time = 0;
	//sb->s_jnl_blocks[17]; /* FIXME */

	sb->s_blocks_count_hi = aux_info.len_blocks >> 32;
	sb->s_r_blocks_count_hi = 0;
	sb->s_free_blocks_count_hi = 0;
	sb->s_min_extra_isize = sizeof(struct ext4_inode) -
		EXT4_GOOD_OLD_INODE_SIZE;
	sb->s_want_extra_isize = sizeof(struct ext4_inode) -
		EXT4_GOOD_OLD_INODE_SIZE;
	sb->s_flags = 0;
	sb->s_raid_stride = 0;
	sb->s_mmp_interval = 0;
	sb->s_mmp_block = 0;
	sb->s_raid_stripe_width = 0;
	sb->s_log_groups_per_flex = 0;
	sb->s_kbytes_written = 0;

	for (i = 0; i < aux_info.groups; i++) {
		u64 group_start_block = aux_info.first_data_block + i *
			info.blocks_per_group;
		u32 header_size = 0;
		if (ext4_bg_has_super_block(i)) {
			if (i != 0) {
				queue_data_block((u8 *)sb, info.block_size, group_start_block);
				queue_data_block((u8 *)aux_info.bg_desc,
					aux_info.bg_desc_blocks * info.block_size,
					group_start_block + 1);
			}
			header_size = 1 + aux_info.bg_desc_blocks + aux_info.bg_desc_reserve_blocks;
		}

		aux_info.bg_desc[i].bg_block_bitmap = group_start_block + header_size;
		aux_info.bg_desc[i].bg_inode_bitmap = group_start_block + header_size + 1;
		aux_info.bg_desc[i].bg_inode_table = group_start_block + header_size + 2;

		aux_info.bg_desc[i].bg_free_blocks_count = sb->s_blocks_per_group;
		aux_info.bg_desc[i].bg_free_inodes_count = sb->s_inodes_per_group;
		aux_info.bg_desc[i].bg_used_dirs_count = 0;
	}
}

void ext4_create_resize_inode()
{
	struct block_allocation *reserve_inode_alloc = create_allocation();
	u32 reserve_inode_len = 0;
	unsigned int i;

	struct ext4_inode *inode = ext4_get_inode(EXT4_RESIZE_INO);
	if (inode == NULL) {
		error("failed to get resize inode");
		return;
	}

	for (i = 0; i < aux_info.groups; i++) {
		if (ext4_bg_has_super_block(i)) {
			u64 group_start_block = aux_info.first_data_block + i *
				info.blocks_per_group;
			u32 reserved_block_start = group_start_block + 1 +
				aux_info.bg_desc_blocks;
			u32 reserved_block_len = aux_info.bg_desc_reserve_blocks;
			append_region(reserve_inode_alloc, reserved_block_start,
				reserved_block_len, i);
			reserve_inode_len += reserved_block_len;
		}
	}

	inode_attach_resize(inode, reserve_inode_alloc);

	inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR;
	inode->i_links_count = 1;

	free_alloc(reserve_inode_alloc);
}

/* Allocate the blocks to hold a journal inode and connect them to the
   reserved journal inode */
void ext4_create_journal_inode()
{
	struct ext4_inode *inode = ext4_get_inode(EXT4_JOURNAL_INO);
	if (inode == NULL) {
		error("failed to get journal inode");
		return;
	}

	u8 *journal_data = inode_allocate_data_extents(inode,
			info.journal_blocks * info.block_size,
			info.journal_blocks * info.block_size);
	if (!journal_data) {
		error("failed to allocate extents for journal data");
		return;
	}

	inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR;
	inode->i_links_count = 1;

	journal_superblock_t *jsb = (journal_superblock_t *)journal_data;
	jsb->s_header.h_magic = htonl(JBD2_MAGIC_NUMBER);
	jsb->s_header.h_blocktype = htonl(JBD2_SUPERBLOCK_V2);
	jsb->s_blocksize = htonl(info.block_size);
	jsb->s_maxlen = htonl(info.journal_blocks);
	jsb->s_nr_users = htonl(1);
	jsb->s_first = htonl(1);
	jsb->s_sequence = htonl(1);

	memcpy(aux_info.sb->s_jnl_blocks, &inode->i_block, sizeof(inode->i_block));
}

/* Update the number of free blocks and inodes in the filesystem and in each
   block group */
void ext4_update_free()
{
	unsigned int i;

	for (i = 0; i < aux_info.groups; i++) {
		u32 bg_free_blocks = get_free_blocks(i);
		u32 bg_free_inodes = get_free_inodes(i);

		aux_info.bg_desc[i].bg_free_blocks_count = bg_free_blocks;
		aux_info.sb->s_free_blocks_count_lo += bg_free_blocks;

		aux_info.bg_desc[i].bg_free_inodes_count = bg_free_inodes;
		aux_info.sb->s_free_inodes_count += bg_free_inodes;

		aux_info.bg_desc[i].bg_used_dirs_count += get_directories(i);
	}
}


