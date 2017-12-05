#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "ext2.h"

unsigned char *disk;

int block_bitmap_alter(unsigned char *disk, int block_num){
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + 1024);
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 1024*2);
	unsigned char *block_bitmap = (unsigned char *)(disk + 1024*(gd->bg_block_bitmap));

	unsigned int byte_offset = (block_num-1) / 8;
	unsigned int bit_offset = (block_num-1) % 8;

	int cur_bit = (block_bitmap[byte_offset]&(1<<bit_offset))>>bit_offset;
	if (!cur_bit){
		block_bitmap[byte_offset]|=(1<<bit_offset);
		gd->bg_free_blocks_count --;
		super_block->s_free_blocks_count --;
		return 1;
	}
	return 0;
}

int inode_bitmap_alter(unsigned char *disk, int block_num){
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + 1024);
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 1024*2);
	unsigned char *inode_bitmap = (unsigned char *)(disk + 1024*(gd->bg_inode_bitmap));

	unsigned int byte_offset = (block_num) / 8;
	unsigned int bit_offset = (block_num) % 8;

	int cur_bit = (inode_bitmap[byte_offset]&(1<<bit_offset))>>bit_offset;

	if (!cur_bit){
		inode_bitmap[byte_offset]|=(1<<bit_offset);
		gd->bg_free_inodes_count --;
		super_block->s_free_inodes_count --;
		return 1;
	}
	return 0;
}

int free_bit_count(unsigned char *disk, int block_num){
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + 1024);
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 1024*2);

	int bit_count = 0;
	if (block_num == 32) {
		unsigned char *inode_bitmap = (unsigned char *)(disk + 1024*(gd->bg_inode_bitmap));
		int i;
		for(i =0;i< block_num; i++){
			unsigned int byte_offset = (block_num) / 8;
			unsigned int bit_offset = (block_num) % 8;
			int cur_bit = (inode_bitmap[byte_offset]&(1<<bit_offset))>>bit_offset;
			if (!cur_bit){
				bit_count++;
			}
		}
	} else if (block_num == 128) {
		unsigned char *block_bitmap = (unsigned char *)(disk + 1024*(gd->bg_block_bitmap));
		int i;
		for(i =0;i< block_num; i++){
			unsigned int byte_offset = (block_num) / 8;
			unsigned int bit_offset = (block_num) % 8;
			int cur_bit = (block_bitmap[byte_offset]&(1<<bit_offset))>>bit_offset;
			if (!cur_bit){
				bit_count++;
			}
		}
	}
	return bit_count;
}


int check_bitmap(unsigned char *disk){
  int total_fixes = 0;
	int free_inode_count = free_bit_count(disk, 32);
	int free_block_count = free_bit_count(disk, 128);
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + 1024);
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 1024*2);
	printf("free_inode_count is %d\n", free_inode_count);
	printf("free_block_count is %d\n", free_block_count);

	// if (/* condition */) {
  //
  //   printf("Fixed: block group's free blocks counter was off by %d compared to the bitmap\n",(128 - used_block_count));
  // }
  //
  //
  //   printf("Fixed: block group's free inodes counter was off by %d compared to the bitmap\n", (32 - used_inode_count));
  // }
  //
  //
  // // NOTE :super_block count
  // used_block_count = 0;
  //
  // if (used_block_count != (128 - super_block->s_free_blocks_count)){
  //
  //   printf("Fixed: superblock's free blocks counter was off by %d compared to the bitmap\n",(128 - used_block_count));
  // }
  //
  //
  // used_inode_count = 0;
  //
  // if (used_inode_count != (32 - super_block->s_free_inodes_count)){
  //
  //   printf("Fixed: superblock's free inodes counter was off by %d compared to the bitmap\n", (32 - used_inode_count));
  // }
  //
  // return total_fixes;
}


int check_inode(unsigned char *disk){
  struct ext2_super_block *super_block = (struct ext2_super_block*)(disk + EXT2_BLOCK_SIZE);
  struct ext2_group_desc *group_desc = (struct ext2_group_desc*)(disk + EXT2_BLOCK_SIZE*2);

	struct ext2_inode *inodes = (struct ext2_inode*)(disk + EXT2_BLOCK_SIZE * group_desc->bg_inode_table);;
	int total_fixes = 0;

	struct ext2_dir_entry *entry;
	int inode_index = 0;



	//NOTE inode bitmap
	while(inode_index < super_block->s_inodes_count){
		if (inode_index == 1 || inode_index > super_block->s_first_ino - 2){
			if(inodes[inode_index].i_blocks != 0){
				if((inodes[inode_index].i_mode & EXT2_S_IFDIR) || (inodes[inode_index].i_mode & EXT2_S_IFREG) || (inodes[inode_index].i_mode & EXT2_S_IFLNK)){
					if (inode_bitmap_alter(disk, inode_index) == 1){
						printf("Fixed: inode [%d] not marked as in-use\n", inode_index);
						total_fixes += 1;
					}

					//NOTE fix i_dtime
					if(inodes[inode_index].i_dtime != 0){
						inodes[inode_index].i_dtime = 0;
						printf("Fixed: valid inode marked for deletion: [%d]\n", inode_index);
						total_fixes += 1;
					}

					// int data_block_unmarked = 0;
					if(inodes[inode_index].i_mode & EXT2_S_IFDIR){
						int p = 0 ;
							while( p < (inodes[inode_index].i_blocks/2) ){
								if (inodes[inode_index].i_block[p] != 0){
								int cur_block_num;
								if (p < 12){
									cur_block_num = inodes[inode_index].i_block[p];
								} else if (p == 12){
									continue;
								} else {
									cur_block_num = (int)(disk + inodes[inode_index].i_block[12] + (p-13));
								}
								entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE * cur_block_num);
								int read = 0;
								while((read < 1024) && (entry->rec_len != 0)){
									read += entry->rec_len;
									int cur_inode_num = entry->inode;
									if (cur_inode_num == 0){
										continue;
									}
									// create cur inode using cur inode number

									struct ext2_inode *cur_inode = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * (group_desc->bg_inode_table) + sizeof(struct ext2_inode) * (cur_inode_num-1));
									//NOTE i_mode
									if(cur_inode->i_mode >= EXT2_S_IFLNK){
										if((cur_inode->i_mode & EXT2_FT_SYMLINK) == EXT2_FT_SYMLINK){
											total_fixes += 1;
											entry->file_type = EXT2_FT_SYMLINK;
											printf("Fixed: Entry type vs inode mismatch: inode [%d]\n", inode_index);
										}
									} else if((cur_inode->i_mode & EXT2_S_IFREG) == EXT2_S_IFREG){
										if(entry->file_type != EXT2_FT_REG_FILE){
											entry->file_type = EXT2_FT_REG_FILE;
											printf("Fixed: Entry type vs inode mismatch: inode [%d]\n", inode_index);
											total_fixes += 1;
										}
									} else if((cur_inode->i_mode & EXT2_S_IFDIR) == EXT2_S_IFDIR){
										if(entry->file_type != EXT2_FT_DIR){
											entry->file_type = EXT2_FT_DIR;
											printf("Fixed: Entry type vs inode mismatch: inode [%d]\n", inode_index);
											total_fixes += 1;
										}
									}

									//NOTE i_block
									if(cur_inode->i_blocks != 0){
										int sub_i;
										int fix_block = 0;
										for ( sub_i = 0 ; sub_i < cur_inode->i_blocks/2; sub_i++){
											int sub_cur_num;
											if(sub_i < 12){
													sub_cur_num = cur_inode->i_block[sub_i];
											}else if (sub_i == 12){
													continue;
											}else{
												sub_cur_num = (int)( disk + cur_inode -> i_block[12] + ( sub_i - 13 ) );
											}

											if(block_bitmap_alter(disk, sub_cur_num) == 1){
												fix_block += 1;
												total_fixes++;
											}
										}
										if (fix_block > 0){
											printf("Fixed: %d in-use data blocks not marked in data bitmap for inode:[%d]\n", fix_block, cur_inode_num);
										}
									}
									// Move to next directory Entry
									entry = (struct ext2_dir_entry *)((char *)entry + entry->rec_len);
								}
							}
							p++;
						}
					}
				}
			}
		}
		inode_index++;
	}
	return total_fixes;
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "Usage: <disk name> <absolute_path>\n");
    exit(1);
  }

	int fd = open(argv[1], O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	check_bitmap(disk);

	int total_fixes = 0;
	total_fixes += check_inode(disk);
	if (total_fixes != 0) {
		printf("%d file system inconsistencies repaired!\n",total_fixes);
	} else {
		printf("No file system inconsistencies detected!");
	}
  return 0;
}
