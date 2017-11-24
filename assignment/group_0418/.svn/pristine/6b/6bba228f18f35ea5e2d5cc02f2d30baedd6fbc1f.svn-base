#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

//used to record the index of the current oldest frame
int oldest;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {

	int chosen_index;

	//set chosen index to the oldest.
	chosen_index = oldest;

	//update oldest.
	if ((oldest + 1) == memsize){
		oldest = 0;
	} else {
		oldest++;
	}

	return chosen_index;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	// no need to do anything in FIFO.
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	oldest = 0;
}
