#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int time;
// to store the frames for comparison.
int *frame_time_record;


/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {

	int chosen_one = 0;
	
	//find the one that is referenced longest time ago
	int count = 0;
	while (count < memsize){
		if (frame_time_record[count] < frame_time_record[chosen_one]){
            chosen_one = count;
      }
      count ++;
	}
   
   // this index of frame is going to be evicted
   // make number to 0 to make the next one start again.
   frame_time_record[chosen_one] = 0;
	return chosen_one;
	
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	// get the index of p and update it in the frame_time_record.
	int this_page = p->frame >> PAGE_SHIFT;
	frame_time_record[this_page] = time;
   time ++;

	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {

	time = 0;
	//malloc the array to store information
	if ((frame_time_record = malloc(memsize * sizeof(int))) == NULL){
		perror("malloc");
      exit(1);
	}
   //initialize the time record array
   int count = 0;
   while (count < memsize){
   	frame_time_record[count] = 0;
   	count ++;
   }

}
