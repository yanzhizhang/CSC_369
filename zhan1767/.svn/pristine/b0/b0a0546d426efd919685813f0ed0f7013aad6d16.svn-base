#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "list.h"

/* Add synchronization code so that operation on the linked list
 * are protected by the lock found in the list struct.

 * You only need to add locking/synchronization to insert.

 * You do not need to add synchronization to length because it is never
 * called by multiple threads, only by main after the threads are finished.
 */

struct node *head = NULL;

struct node *create_node(int value) {
    struct node *newnode = malloc(sizeof(struct node));
    newnode->value = value;
    newnode->next = NULL;
    return newnode;
}


void insert(struct list *L, int value){
    struct node *newnode = create_node(value);

    pthread_mutex_lock(&L->lock);

    struct node *cur = L->head;

    if(L->head == NULL) {
		L->head = newnode;
        pthread_mutex_unlock(&L->lock);
        return;
    } else if(L->head->value >value) {
        newnode->next = L->head;
		L->head = newnode;
        pthread_mutex_unlock(&L->lock);
        return;
    }

    while(cur->next != NULL && cur->next->value <= value) {
        cur = cur->next;
    }

    newnode->next = cur->next;
    cur->next = newnode;
    // head doesn't change
    pthread_mutex_unlock(&L->lock);
    return;
}

int length(struct list *L) {
	struct node *cur = L->head;
	int count = 0;
	while(cur != NULL) {
		count++;
		cur = cur->next;
	}
	return count;
}

void print_list(struct list *L) {
	struct node *cur = L->head;
    while(cur != NULL) {
        printf("%d -> ", cur->value);
        cur = cur->next;
    }
    printf("\n");
}
