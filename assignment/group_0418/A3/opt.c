#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

// global foropt
// added frame.frame_time_record field for record of time
int current_instruction;
int total_instruction_num;
addr_t* instruction_array;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	int longest_wait;
	int chosen_one = 0;
	longest_wait = coremap[0].frame_time_record;
	
	// find the one that will not be referenced for the longest time.
	int current = 1;
	while(current < memsize){
		if (coremap[current].frame_time_record > longest_wait){
			longest_wait = coremap[current].frame_time_record;
			chosen_one = current;
		}
		current ++;	
	}
	return chosen_one;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	
	int frame_number;
	frame_number = p->frame >> PAGE_SHIFT; 
	
	// when referenced, make every frame in the array closer to the end
	// frame_time_record might be negative if no address in such frame.
	int count = 0;
	while (count < memsize){
		coremap[count].frame_time_record --;
		count ++;
	}
	
	//update the frame that is being referenced
	coremap[frame_number].frame_time_record = 1;
	
	//start counting to see where the same address will be referenced.
	//if no referenced after, then the time will be the same as length of instruction number then such frame will be the choice to replace next.
	int count2 = current_instruction + 1;
	while ((count2 < total_instruction_num) && (instruction_array[count2] != instruction_array[current_instruction])){
		coremap[frame_number].frame_time_record ++;
		count2 ++;
	}
	
	// indicate that we are read one instruction, move forward to the next one.
	current_instruction ++;
	
	// if we already have all the instruction read, free arrays.
	if (current_instruction == total_instruction_num){
		free(instruction_array);
	}
	
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	
	total_instruction_num = 0;
	current_instruction = 0;
	FILE* traces;
	char useful[512];
	char useless;
	
	traces = fopen(tracefile, "r");
	if (traces == NULL){
		perror("fopen");
		exit(1);
	}
	
	//get the number of instruction in total
	while(fgets(useful, 512, traces)){
		// the first few line of trace files does not count.
		if(useful[0] != '='){
			total_instruction_num++;
		}
	}
	fclose(traces);
	
	// initialize the added fields from coremap.
	int count = 0;
	while (count < memsize){
		coremap[count].frame_time_record = 0;
		count ++;
	}
	
	//malloc the array to record the memory address
	instruction_array = malloc(total_instruction_num * sizeof(addr_t));
	if (instruction_array == NULL){
		perror("malloc");
		exit(1);
	}
	
	// reopen to read from the beginning
	traces = fopen(tracefile, "r");
	if (traces == NULL){
		perror("fopen");
		exit(1);
	}
	
	// read from tracefile and store the insturction memory address for further "prediction"
	int count2 = 0;
	while(fgets(useful, 512, traces)){
		if(useful[0] != '='){
			sscanf(useful, "%c %lx", &useless, &instruction_array[count2]);
			count2 ++;
		}
	}
	fclose(traces);

}

