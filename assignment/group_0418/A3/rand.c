#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "sim.h"
#include "pagetable.h"



extern struct frame *coremap;

/* Page to evict is chosen using the rand algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int rand_evict() {
	// choose index in coremap to evict a page from
	int idx = (int)(random() % memsize);
	
	return idx;
}

/* This function is called on each access to a page to update any information
 * needed by the rand algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void rand_ref(pgtbl_entry_t *p) {

	return;
}

void rand_init() {
}
