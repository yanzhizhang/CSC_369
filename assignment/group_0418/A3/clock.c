#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int pointer;

int clock_evict() {

	int chosen_one;

	while (1){
		// if the frame is no second chance.
		if (!(PG_REF & coremap[pointer].pte->frame)){
			
			chosen_one = pointer;
			if ((pointer + 1) == memsize){
				pointer = 0;
			} else {
				pointer++;
			}
			return chosen_one;

		}else{
			// the frame used the second chance
			// set the reference bit to 0;
			coremap[pointer].pte->frame &= ~PG_REF;
			if ((pointer + 1) == memsize){
				pointer = 0;
			} else {
				pointer++;
			}
		}
	}

	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// set the "second chance" token
	p->frame = p->frame | PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	pointer = 0;
}
