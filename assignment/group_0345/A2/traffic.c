#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "traffic.h"

struct intersection isection;

/*
 *
 * Prints the state of each lanes out_cars. This code is used
 * for testing purposes and should not be removed or modified.
 *
 */
void verify() {
    int i;
    struct car *cur;

    printf("---\n");

    /* iterate through lanes */
    for (i = 0; i < 4; i++) {

        /* iterate through all cars in the lanes out_cars */
        for (cur = isection.lanes[i].out_cars; cur != NULL; cur = cur->next) {
            printf("%d %d %d\n", cur->in_dir, cur->out_dir, cur->id);
        }
    }
    
	printf("===\n");
}

int main(int argc, char *argv[]) {
    int i;
    pthread_t in_threads[4], cross_threads[4];

    if (argc != 2) {
        printf("Usage: %s <schedules_file>\n", argv[0]);
        exit(1);
    }

    init_intersection();
    parse_schedule(argv[1]);

    /* spin up threads */
    for (i = 0; i < 4; i++) {
        pthread_create(&cross_threads[i], NULL, &car_cross, (void *) &isection.lanes[i]);
        pthread_create(&in_threads[i], NULL, &car_arrive, (void *) &isection.lanes[i]);
    }

    /* wait for all arrival threads */
    for (i = 0; i < 4; i++) {
        if (pthread_join(cross_threads[i], NULL)) {
            exit(1);
        }
    }

    /* wait for all crossing threads */
    for (i = 0; i < 4; i++) {
        if (pthread_join(in_threads[i], NULL)) {
            exit(1);
        }
    }

	verify();

    return 0;
}
