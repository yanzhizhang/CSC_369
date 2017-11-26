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
#define EISDIR 21 /* Is a Directory*/

unsigned char *disk;
struct ext2_super_block *super_block;
struct ext2_group_desc *group_desc;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
struct ext2_inode *inodes;
unsigned int inode_index_given_name_parent(struct ext2_inode *parent_inode, char* name, unsigned char type);
struct ext2_inode* inode_given_path(char *input);

struct ext2_inode* inode_given_path(char *input){
	char real_path[1024];
	int path_len = strlen(input);
	strcpy(real_path, input);
	real_path[path_len + 1] = '\0';
	
	unsigned int num;
	
	// make the parent inode point to the root
	struct ext2_inode *parent_inode = &inodes[EXT2_ROOT_INO - 1];
	if (path_len != 1){
		char* temp = strtok(real_path, "/");
		while (temp != NULL){
			printf("the temp : %s \n", temp);
			num = inode_index_given_name_parent(parent_inode, temp, EXT2_FT_DIR);
			printf("inode num : %d\n", num);
			if (num == 0){
				return NULL;
			}
			parent_inode = &inodes[num-1];
			temp = strtok(NULL, "/");
		}
	}
	else if (path_len == 1){
		if (real_path[0] == '/'){
			return parent_inode;		
		}	
	}
	return parent_inode;
}

unsigned int inode_index_given_path(char* input){
	
	if (strlen(input) == 1){
		if(input[0] == '/'){
			return EXT2_ROOT_INO ;		
		}	
	}
	
	char real_path[1024];
	int path_len = strlen(input);
	strcpy(real_path, input);
	real_path[path_len + 1] = '\0';
	unsigned int num = 0;

	struct ext2_inode *parent_inode = &inodes[EXT2_ROOT_INO - 1];
	if (path_len != 1){
		char* temp = strtok(real_path, "/");
		while (temp != NULL){
			num = inode_index_given_name_parent(parent_inode, temp, EXT2_FT_DIR);
			if (num == 0){
				return -1;
			}
			parent_inode = &inodes[num-1];
			temp = strtok(NULL, "/");
		}
	}
	else if (path_len == 1){
		if (real_path[0] == '/'){
			return num;		
		}	
	}
	return num;
}

unsigned int inode_index_given_name_parent(struct ext2_inode *parent_inode, char* name, unsigned char type){
	
	struct ext2_dir_entry *entry;
	int count = 0;
   int memory_count;
   int flag1 = 0;
   int flag2 = 0; 
   int flag3 = 0;
   
   // loop through the inode's i_block
   while (count < 15){
   	if (parent_inode->i_block[count]!=0){
   		memory_count = 0;
   		entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE * parent_inode->i_block[count]);
   		while(memory_count < EXT2_BLOCK_SIZE){
   			if (strlen(name) == entry->name_len){
   				flag1 = 1;
   			}
   			if (strncmp(name, entry->name, entry->name_len) == 0){
   				flag2 = 1;
   			}
   			if (type & entry->file_type){
   				flag3 = 1;
   			}
   			if (flag1 == 1 & flag2 == 1 & flag3 == 1){
   				printf("entry->inode %d \n", entry->inode);
   				return entry->inode;
   			}
				flag1 = 0;flag2 = 0;flag3 = 0;
				memory_count += entry->rec_len;
				entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE * parent_inode->i_block[count] + memory_count);
  		 	}
   	}
   	count ++;
   }
   return 0;
}


unsigned int init_inode(){
	unsigned char *pointer = inode_bitmap;
	unsigned char temp;
	int flag1 = 0;
	int flag2 = 0;
	int count = 0;
	int count2 = 0;
	int ind = 1;
	
	while (count < (super_block->s_inodes_count)/sizeof(char*)){
			temp = *pointer;
			while (count2 < 8) {
				
				if(ind >= 11){
					flag1 = 1;				
				}
				
				if(!((temp>>count2) & 1)){
					flag2 = 1;
				}
				
				if (flag1 & flag2){
					printf("New Inode Index: %d\n", ind);
					return ind;	
				}
				
				flag1 = 0;flag2 = 0;
				count2 ++;
				ind++;
			}
			count2 = 0;
			count ++;
			pointer++;
	}
	return 0;
}

unsigned int init_block(){
	unsigned char *pointer = block_bitmap;
	unsigned char temp;
	int count = 0;
	int count2 = 0;
	int ind = 1;
	int section_num = (super_block->s_blocks_count)/sizeof(char*);
	while (count < section_num ){
			temp = *pointer;

			while (count2 < 8) {
				if(!((temp>>count2) & 1)){
					printf("New Block index : %d\n", ind);
					return ind;
				}
				ind ++;
				count2 ++;
			}
			count2 = 0;
			count ++;
			pointer++;
	}
	return 0;
}

void block_bitmap_alter(unsigned int ind, int result){
	int off = ind / (sizeof(char*));
	int rem = ind % (sizeof(char*));
	
	unsigned char* position = block_bitmap + off;
	*position = ((*position & ~(1 << rem)) | (result << rem));
}

void inode_bitmap_alter(unsigned int ind, int result){
	int off = ind / (sizeof(char*));
	int rem = ind % (sizeof(char*));
	
	unsigned char* position = inode_bitmap + off;
	*position = ((*position & ~(1 << rem)) | (result << rem));
}

int alter_inode(unsigned int ind, unsigned short mode, unsigned int size, unsigned int new_block){
	
	//subject to change
	inodes[ind].i_mode = mode;
	inodes[ind].i_uid = 0;
	inodes[ind].i_gid = 0;
	inodes[ind].i_dtime = 0;
	inodes[ind].osd1 = 0;
	inodes[ind].i_links_count = 0;
	inodes[ind].i_blocks = 2;
	inodes[ind].i_size = size;
	inodes[ind].i_block[0] = 1 + new_block;
	inodes[ind].i_generation = 0;
	inodes[ind].i_file_acl = 0;
	inodes[ind].i_dir_acl = 0;
	inodes[ind].i_faddr = 0;
	inodes[ind].extra[0] = 0;
	inodes[ind].extra[1] = 0;
	inodes[ind].extra[2] = 0;
	return 0;
}

int get_entry_size(char* name){
	int result = 8;
	if((strlen(name)%4) != 0){
		result += (strlen(name)/4)*4 + 4;	
	}else{
		result += strlen(name);
	}
	
	return result;
}

struct ext2_dir_entry * add_dir_entry(unsigned char file_type, struct ext2_inode *parent_inode, struct ext2_dir_entry* new_entry, char* new_name){

	int memory_record;
	int actual_rec_len;
	struct ext2_dir_entry *entry;
	int dir_size = sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned char)*2;
	for (int i = 0; i < parent_inode->i_blocks/2; i ++){
		memory_record = 0;
		while (memory_record < EXT2_BLOCK_SIZE){
			entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE*parent_inode->i_block[i] + memory_record);
			if (memory_record + entry->rec_len == EXT2_BLOCK_SIZE){
				entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE*parent_inode->i_block[i] + memory_record);
				actual_rec_len = get_entry_size(entry->name);
				if(actual_rec_len + new_entry->rec_len < entry->rec_len){
					entry->rec_len = actual_rec_len;
					// move to the
					entry = (struct ext2_dir_entry*)(disk + EXT2_BLOCK_SIZE*parent_inode->i_block[i] + memory_record + actual_rec_len);
					entry-> inode = new_entry->inode;
					entry->rec_len = EXT2_BLOCK_SIZE - actual_rec_len - memory_record;
					strcpy(entry->name, new_name);
					entry->name_len = strlen(new_name);
					entry->name[strlen(entry->name)] = '\0';
					entry->file_type = file_type;	
					parent_inode->i_links_count++;		
					return entry;
				}				
			}
			memory_record += entry->rec_len;
		}	
	}
	return NULL;		
}

int check_inter(char* parent_path){
	if (strlen(parent_path) == 1){
		if(parent_path[0] == '/'){
			return 0;		
		}	
	}
	
	char copy[1024];
	strcpy(copy, parent_path);
	char* temp = strtok(copy, "/");
	struct ext2_inode *parent_inode = (struct ext2_inode*)(inodes + ((EXT2_ROOT_INO-1)*sizeof(struct ext2_inode)));
	int num;
	while (temp != NULL){
		num = inode_index_given_name_parent(parent_inode, temp, EXT2_FT_DIR);
		parent_inode = &inodes[num-1];
		temp = strtok(NULL, "/");
		if (num == 0 & temp != NULL){
			return -1;
		}
	}
	return 0;
}

int main(int argc, char **argv){
	// check argvs

  if ((argc == 5) && (strncmp(argv[2], "-s", 2) != 0)){
    fprintf(stderr, "Usage: <disk>  -s[optional] <absolute path> <another absolute path>\n");
		exit(1);
  }
  if (!((argc == 4) || (argc == 5))){
		fprintf(stderr, "Usage: <disk>  -s[optional] <absolute path> <another absolute path>\n");
		exit(1);
  }

  char* chosen_disk = argv[1];
  // initialize the disk
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
  
  //for hardlink
  if (argc == 4){
  		
  		char* chosen_source = argv[2];
  		char* chosen_dest = argv[3];
  		
  		//getting source
  		char* real_source = malloc(256); 
    	int s_path_len = strlen(chosen_source);
    	char* s_parent_path;
	 	if (chosen_source[s_path_len - 1] == '/'){
    		strcpy(real_source, chosen_source);
    		real_source[s_path_len + 1] = '\0';
    		real_source[s_path_len + 2] = '\0';
    	} else if (chosen_source[s_path_len - 1] != '/'){
    		strcpy(real_source, chosen_source);
    		strcat(real_source, "/");
    		real_source[s_path_len + 1] = '\0';
   	}
    	
    	//start getting the parent's inode and other information.
    	int k = strlen(real_source) - 2;
    	while (k > -1 & real_source[k]!='/'){
    			k --;
    	}

    	if (k < 0){
    		s_parent_path = malloc(2);
    		strcpy(s_parent_path, "/");
    		s_parent_path[1] = '\0';
   	}else{
    		s_parent_path = malloc(k+1);
    		strncpy(s_parent_path, real_source, k+1);
    		s_parent_path[k+1] = '\0';
    	}
    	
    	//get destination.
    	char* real_dest = malloc(256); 
    	int d_path_len = strlen(chosen_dest);
    	char* d_parent_path;
	 	if (chosen_dest[d_path_len - 1] == '/'){
    		strcpy(real_dest , chosen_dest);
    		real_dest [d_path_len + 1] = '\0';
    		real_dest [d_path_len + 2] = '\0';
    	} else if (chosen_dest[d_path_len - 1] != '/'){
    		strcpy(real_dest, chosen_dest);
    		strcat(real_dest, "/");
    		real_dest[d_path_len + 1] = '\0';
   	}
   	
   	struct ext2_inode* dest_inode = inode_given_path(real_dest);
    	if (dest_inode != NULL){
    		return EEXIST;  	
    	}
    	
    	//start getting the parent's inode and other information.
    	k = strlen(real_dest) - 2;
    	while (k > -1 & real_dest[k]!='/'){
    			k --;
    	}

    	if (k < 0){
    		d_parent_path = malloc(2);
    		strcpy(d_parent_path, "/");
    		d_parent_path[1] = '\0';
   	}else{
    		d_parent_path = malloc(k+1);
    		strncpy(d_parent_path, real_dest, k+1);
    		d_parent_path[k+1] = '\0';
    	}
    	
    	char* self_source_name = malloc(256);
    	char copy_path[1024];
    	strcpy(copy_path, real_source);
    	char* temp = strtok(copy_path, "/");
	 	while (temp != NULL){
	 		strcpy(self_source_name, temp);
			temp = strtok(NULL, "/");
	 	}
	 	self_source_name[strlen(self_source_name)] = '\0';
	 	
	 	char* self_dest_name = malloc(256);
    	char copy_path2[1024];
    	strcpy(copy_path2, real_dest);
    	char* temp2 = strtok(copy_path2, "/");
	 	while (temp2 != NULL){
	 		strcpy(self_dest_name, temp2);
			temp2 = strtok(NULL, "/");
	 	}
	 	self_dest_name[strlen(self_dest_name)] = '\0';
    	
    	printf("real source : %s \n", real_source);
    	printf("real dest : %s \n", real_dest);
    	printf("source parent : %s \n", s_parent_path);
    	printf("dest parent : %s \n", d_parent_path);
    	printf("source name : %s \n", self_source_name);
    	printf("dest name : %s \n", self_dest_name);
    	
    	
    	struct ext2_inode * source_parent_inode = inode_given_path(s_parent_path);
    	struct ext2_inode * dest_parent_inode = inode_given_path(d_parent_path);
    	
    	int source_inode_index = inode_index_given_name_parent(source_parent_inode, self_source_name, EXT2_FT_REG_FILE);
    	if(source_inode_index == 0){
    		printf("Source Not Exist! \n");
			return ENOENT;    	
    	}
    	
    	if(inode_index_given_name_parent(dest_parent_inode, self_dest_name, EXT2_FT_REG_FILE) != 0){
    		printf("Dest Already Exist! \n");
			return EEXIST;    	
    	}
    	
    	struct ext2_dir_entry* new_entry = malloc(sizeof(struct ext2_dir_entry));
    	new_entry -> inode = source_inode_index;
    	new_entry -> file_type = EXT2_FT_REG_FILE;
    	new_entry->name_len = strlen(self_dest_name);
    	new_entry -> rec_len = get_entry_size(self_dest_name);
    	
    	add_dir_entry(EXT2_FT_REG_FILE, dest_parent_inode, new_entry, self_dest_name);
    	
    	inodes[source_inode_index - 1].i_links_count ++;
    	
    	free(new_entry);	
  		free(self_dest_name);
  		free(self_source_name);
  		free(real_source);
  		free(real_dest);
  		free(s_parent_path);
  		free(d_parent_path);
 
  		return 0;
  
  }
  
  // for softlink
  if (argc == 5){
  	
  		char* chosen_source = argv[3];
  		char* chosen_dest = argv[4];
  		
  		//getting source
  		char* real_source = malloc(256); 
    	int s_path_len = strlen(chosen_source);
    	char* s_parent_path;
	 	if (chosen_source[s_path_len - 1] == '/'){
    		strcpy(real_source, chosen_source);
    		real_source[s_path_len + 1] = '\0';
    		real_source[s_path_len + 2] = '\0';
    	} else if (chosen_source[s_path_len - 1] != '/'){
    		strcpy(real_source, chosen_source);
    		strcat(real_source, "/");
    		real_source[s_path_len + 1] = '\0';
   	}
    	
    	//start getting the parent's inode and other information.
    	int k = strlen(real_source) - 2;
    	while (k > -1 & real_source[k]!='/'){
    			k --;
    	}

    	if (k < 0){
    		s_parent_path = malloc(2);
    		strcpy(s_parent_path, "/");
    		s_parent_path[1] = '\0';
   	}else{
    		s_parent_path = malloc(k+1);
    		strncpy(s_parent_path, real_source, k+1);
    		s_parent_path[k+1] = '\0';
    	}
    	
    	//get destination.
    	char* real_dest = malloc(256); 
    	int d_path_len = strlen(chosen_dest);
    	char* d_parent_path;
	 	if (chosen_dest[d_path_len - 1] == '/'){
    		strcpy(real_dest , chosen_dest);
    		real_dest [d_path_len + 1] = '\0';
    		real_dest [d_path_len + 2] = '\0';
    	} else if (chosen_dest[d_path_len - 1] != '/'){
    		strcpy(real_dest, chosen_dest);
    		strcat(real_dest, "/");
    		real_dest[d_path_len + 1] = '\0';
   	}
   	
   	struct ext2_inode* dest_inode = inode_given_path(real_dest);
    	if (dest_inode != NULL){
    		return EEXIST;  	
    	}
    	
    	//start getting the parent's inode and other information.
    	k = strlen(real_dest) - 2;
    	while (k > -1 & real_dest[k]!='/'){
    			k --;
    	}

    	if (k < 0){
    		d_parent_path = malloc(2);
    		strcpy(d_parent_path, "/");
    		d_parent_path[1] = '\0';
   	}else{
    		d_parent_path = malloc(k+1);
    		strncpy(d_parent_path, real_dest, k+1);
    		d_parent_path[k+1] = '\0';
    	}
    	
    	char* self_source_name = malloc(256);
    	char copy_path[1024];
    	strcpy(copy_path, real_source);
    	char* temp = strtok(copy_path, "/");
	 	while (temp != NULL){
	 		strcpy(self_source_name, temp);
			temp = strtok(NULL, "/");
	 	}
	 	self_source_name[strlen(self_source_name)] = '\0';
	 	
	 	char* self_dest_name = malloc(256);
    	char copy_path2[1024];
    	strcpy(copy_path2, real_dest);
    	char* temp2 = strtok(copy_path2, "/");
	 	while (temp2 != NULL){
	 		strcpy(self_dest_name, temp2);
			temp2 = strtok(NULL, "/");
	 	}
	 	self_dest_name[strlen(self_dest_name)] = '\0';
    	
    	printf("real source : %s \n", real_source);
    	printf("real dest : %s \n", real_dest);
    	printf("source parent : %s \n", s_parent_path);
    	printf("dest parent : %s \n", d_parent_path);
    	printf("source name : %s \n", self_source_name);
    	printf("dest name : %s \n", self_dest_name);
    	
    	struct ext2_inode * source_parent_inode = inode_given_path(s_parent_path);
    	struct ext2_inode * dest_parent_inode = inode_given_path(d_parent_path);
    	
    	int hardlink_inode_index = init_inode();
    	int hardlink_block_index = init_block();
    	
    	super_block->s_free_blocks_count --;
    	super_block->s_free_inodes_count --;
    	group_desc->bg_free_blocks_count --;
    	group_desc->bg_free_inodes_count --;
    	
    	block_bitmap_alter(hardlink_block_index - 1, 1);
    	inode_bitmap_alter(hardlink_block_index - 1, 1);
    
    	alter_inode(hardlink_inode_index - 1, EXT2_S_IFLNK, strlen(real_source) - 1, hardlink_block_index);
    	struct ext2_dir_entry* new_entry = malloc(sizeof(struct ext2_dir_entry));
    	new_entry -> inode = hardlink_inode_index;
    	new_entry -> file_type = EXT2_FT_SYMLINK;
    	new_entry->name_len = strlen(self_dest_name);
    	new_entry -> rec_len = get_entry_size(self_dest_name);
    	
    	add_dir_entry(EXT2_FT_SYMLINK, dest_parent_inode, new_entry, self_dest_name);
    	char *block_bit = (char*)(disk + inodes[hardlink_inode_index - 1].i_block[0]*EXT2_BLOCK_SIZE);
    	
    	for(int m = 0; m < strlen(real_source); m++){
    		*block_bit = real_source[m];
    		block_bit++;
    	}
    	*block_bit = '\0';
    	
    	free(new_entry);	
  		free(self_dest_name);
  		free(self_source_name);
  		free(real_source);
  		free(real_dest);
  		free(s_parent_path);
  		free(d_parent_path);
    	
    	return 0;	
  }


}
