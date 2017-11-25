#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>
#define EEXIST 17 /* File exists */
#define ENOENT 2  /* No such file or directory */

unsigned char *disk;
struct ext2_super_block *super_block;
struct ext2_group_desc *group_desc;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
struct ext2_inode *inodes;
unsigned int inode_index_given_name_parent(struct ext2_inode *parent_inode, char* name, unsigned char type);
struct ext2_inode* inode_given_path(char *input);

int delete_dir_entry(struct ext2_inode *parent_inode, char* self_name){
	int memory_record;
	struct ext2_dir_entry *entry;
	int removed_inode;
	int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0;

	for (int i = 0; i < parent_inode->i_blocks/2; i++){
		memory_record = 0;
		while (memory_record < EXT2_BLOCK_SIZE){
			entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE*parent_inode->i_block[i] + memory_record);
			if (entry->file_type != (unsigned char)EXT2_FT_DIR) {
				flag1 = 1;
			}
			if (strlen(self_name)==strlen(entry->name)){
				flag2 = 1;
			}
			if (strncmp(self_name,entry->name,entry->name_len)==0){
				flag3 = 1;
			}
			if (entry->inode != 0){
				flag4 = 1;
			}
			if (flag1 & flag2 & flag3 & flag4){
				removed_inode = entry->inode;
				entry->inode = 0;
				return removed_inode;
			}
			memory_record += entry->rec_len;
	}
	return 0;
}

int complete_remove(struct ext2_inode * removed_inode){

	// remove all the direct link
	if ((removed_inode -> i_blocks)/2 < 12){
		for(i = 0; i < (removed_inode-> i_blocks)/2; i++){  
            super_block->s_free_blocks_count ++;
            group_desc->bg_free_blocks_count ++;
            block_bitmap_alter(removed_inode->i_block[i]-1,0);   
    }

    // remove indirect link
	if (!((removed_inode -> i_blocks)/2) < 12){

		for(i = 0; i < 12; i++){  
            super_block->s_free_blocks_count ++;
            group_desc->bg_free_blocks_count ++;
            block_bitmap_alter(removed_inode->i_block[i]-1,0);  
      }
      

		//TODO
		//implement direct and double indirect
	}

	return 0;
}

int main(int argc, char **argv) {

	if (argc != 3){  
        fprintf(stderr, "Usage: <image file> <absolute file path>\n");  
        return 1;  
    }  

    char* chosen_disk = argv[1];
    char* chosen_path = argv[2];

    int fd = open(chosen_disk, O_RDWR);
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // find useful component
    super_block = (struct ext2_super_block*)(disk + EXT2_BLOCK_SIZE);
    group_desc = (struct ext2_group_desc*)(disk + EXT2_BLOCK_SIZE*2);
    block_bitmap = (unsigned char*)(disk + EXT2_BLOCK_SIZE * group_desc->bg_block_bitmap);
    inode_bitmap = (unsigned char*)(disk + EXT2_BLOCK_SIZE * group_desc->bg_inode_bitmap);
    inodes = (struct ext2_inode*)(disk + EXT2_BLOCK_SIZE * group_desc->bg_inode_table);
    
    char* real_path = malloc(256); 
    int path_len = strlen(chosen_path);
	 if (chosen_path[path_len - 1] == '/'){
    	strcpy(real_path, chosen_path);
    	real_path[path_len + 1] = '\0';
    	real_path[path_len + 2] = '\0';
    } else if (chosen_path[path_len - 1] != '/'){
    	strcpy(real_path, chosen_path);
    	strcat(real_path, "/");
    	real_path[path_len + 1] = '\0';
    }

    int k = strlen(real_path) - 2;
    while (k > -1 & real_path[k]!='/'){
    		k --;
    }
    //get parent directory
    char* parent_path;
    if (k < 0){
    	parent_path = malloc(2);
    	strcpy(parent_path, "/");
    	parent_path[1] = '\0';
    }else{
    	parent_path = malloc(k+1);
    	strncpy(parent_path, real_path, k+1);
    	parent_path[k+1] = '\0';
    }

    //get self name
    char copy_path[1024];
	 char cp[1024];
    strcpy(copy_path, real_path);
    char* temp = strtok(copy_path, "/");
	 while (temp != NULL){
		 strcpy(cp, temp);
		 temp = strtok(NULL, "/");
	 }
	 char* self_name = malloc(strlen(cp));
	 strcpy(self_name, cp);
	 self_name[strlen(cp)] = '\0';

	 struct ext2_inode *parent_inode = inode_given_path(parent_path);
	 if (inode_index_given_name_parent(parent_inode, self_name, EXT2_FT_DIR)!=0){
        fprintf(stderr, "link refers to a DIR\n");
        return EISDIR;
    } 

	 int check = delete_dir_entry(parent_inode, self_name);
	 if (check == 0){
    	 return ENOENT; 
	 }

	 // set removed inode
	 struct ext2_inode * removed_inode = &inodes[check - 1]; 
    removed_inode->i_links_count--;

    if (removed_inode->i_links_count != 0){

    	super_block->s_free_inodes_count ++;  
    	group_desc->bg_free_inodes_count ++;  
    	removed_inode->i_dtime = time(NULL); 
    	inode_bitmap_alter(removed_inode-1, 0); 

    	complete_remove(removed_inode); 

    } else {

    	return 0;
    }

	return 0;

}


