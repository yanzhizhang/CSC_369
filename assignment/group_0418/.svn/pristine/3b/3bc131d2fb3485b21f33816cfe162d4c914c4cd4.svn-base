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

addr_t* instruction_array;
int current_instruction;
int total_instruction_num = 0;
int* frame_time_record;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	int current;
	int longest_wait;
	int chosen_one = 0;
	
	longest_wait = frame_time_record[0];
	
	for (current = 1; current < memsize; current ++){	
		if (frame_time_record[current] > longest_wait){
		
			longest_wait = frame_time_record[current];
			chosen_one = current;
		}	
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
	
	int count;
	
	for (count = 0; count < memsize; count ++){
		frame_time_record[count] --;
	}
	
	frame_time_record[frame_number] = 1;

	int count2;
	
	for (count2 = current_instruction + 1; count2 < total_instruction_num; count2 ++){
		
		if(instruction_array[count2] != instruction_array[current_instruction]){
			frame_time_record[frame_number] ++;
		}
		else if (instruction_array[count2] == instruction_array[current_instruction]){
			break;	
		}
	}
	
	// indicate that we are read one instruction, move forward to the next one.
	current_instruction ++;
	
	// if we already have all the instruction read, free arrays.
	if (current_instruction == total_instruction_num){
		free(frame_time_record);
		free(instruction_array);
	}
	
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	
	current_instruction = 0;
	FILE* traces;
	
	frame_time_record = malloc(memsize * sizeof(int));
	
	if (frame_time_record == NULL){
		perror("malloc");
		exit(1);
	}
	
	traces = fopen(tracefile, "r");
	char buffer[256];
	char useless;
	
	while(fgets(buffer, 256, traces)){
		// the first few line of trace files.
		if(buffer[0] != '='){
			total_instruction_num++;
		}
	}
	
	fclose(traces);
	// reopen to read from the beginning
	traces = fopen(tracefile, "r");
	int count = 0;
	
	instruction_array = malloc(total_instruction_num * sizeof(addr_t));
	if (instruction_array == NULL){
		perror("malloc");
		exit(1);
	}
	
	while(fgets(buffer, 256, traces)){
		if(buffer[0] != '='){
			sscanf(buffer, "%c %lx", &useless, &instruction_array[count]);
			count ++;
		}
	}
	
	for (int i = 0; i < memsize; i++){
		frame_time_record[i] = total_instruction_num + 1;
	}
	
	fclose(traces);
}

