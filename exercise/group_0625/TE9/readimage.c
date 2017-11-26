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
    
    unsigned char byte;
    
    printf("Block bitmap: ");
    unsigned char *block_bitmap = (unsigned char*)(disk + 1024*3);
    int i, j;
    for (i=0 ; i<=15 ; i++){
       for (j=0 ; j<8 ; j++){
           byte = (block_bitmap[i] >> j) & 1;
           printf("%u", byte);
       }
       printf(" ");
    }
    printf("\n");
    
    printf("Inode bitmap: ");
    unsigned char *inode_bitmap = (unsigned char*)(disk + 1024*4);
    int m, n;
    for (m=0 ; m<=3 ; m++){
       for (n=0 ; n<8 ; n++){
           byte = (inode_bitmap[m] >> n) & 1;
           printf("%u", byte);
       }
       printf(" ");
    }
    printf("\n");
    printf("\n");
    printf("Inodes:\n");
    
    struct ext2_inode *inodes = (struct ext2_inode *)(disk + 1024 * 5);
    
    int count = 0;
    char c = 0;
    while(count < 32){
    	if (count == 1 || count > 10){
    		if(inodes[count].i_size != 0){
    			if(inodes[count].i_mode & EXT2_S_IFREG){
					c = 'd'; 
   			}else if (inodes[count].i_mode & EXT2_S_IFDIR){
   				c = 'f';
   			}
    			printf("[%d] type: %c size: %d links: %d blocks: %d\n",count + 1, c, inodes[count].i_size, inodes[count].i_links_count, inodes[count].i_blocks);
    			printf("[%d] Blocks: ",count + 1);
    			int k = 0;
    			while(k<15){
    				if (inodes[count].i_block[k] != 0){
    					printf("%d ", inodes[count].i_block[0]);
    				}
    				k++;
    			}
    			printf("\n");
    		}
    	}
    	count ++;
    }
    printf("\n");
    printf("Directory Blocks:\n");
	 
	 struct ext2_dir_entry *entry;
    int count2 = 0;
    int memory_count;
    char type;
    while(count2 < 32){
    	if (count2 == 1 || count2 > 10){
    		if(inodes[count2].i_size != 0){
    			if(inodes[count2].i_mode & EXT2_S_IFDIR){
    				int p = 0;
    				while(p<15){
    					if (inodes[count2].i_block[p] != 0){
    						printf("   DIR BLOCK NUM: %d (for inode %d)\n", inodes[count2].i_block[p], count2+1);
    						memory_count = 0;
    						entry = (struct ext2_dir_entry*)(disk + 1024*inodes[count2].i_block[p]);
    						while(memory_count < 1024){
    							if(entry->file_type == EXT2_FT_DIR){
									type = 'd'; 
   							}else if (entry->file_type == EXT2_FT_REG_FILE){
   								type = 'f';
   							}
    							printf("Inode: %u rec_len: %u name_len: %u type=%c name=%.*s\n", entry->inode, entry->rec_len, entry->name_len, type, entry->name_len, entry->name);
    							memory_count += entry->rec_len;
    							entry = (struct ext2_dir_entry*)(disk + 1024*inodes[count2].i_block[p] + memory_count);
    						}	
    					}
    					p++;
    				}	
   			}
    		}
    	}
    	count2++;
    }
    

    return 0;
}
