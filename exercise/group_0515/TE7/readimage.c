#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;


int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    struct ext2_group_desc *address = (struct ext2_group_desc *)(disk + 1024*2);
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    printf("Block group:\n");
    printf("    block bitmap: %d\n",address->bg_block_bitmap);
    printf("    inode bitmap: %d\n",address->bg_inode_bitmap);
    printf("    inode table: %d\n",address->bg_inode_table);
    printf("    free blocks: %d\n",address->bg_free_blocks_count);
    printf("    free inodes: %d\n",address->bg_free_inodes_count);
    printf("    used_dirs: %d\n",address->bg_used_dirs_count);
    
    return 0;
}
