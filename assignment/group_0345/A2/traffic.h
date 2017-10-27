#ifndef __TRAFFIC_H__
#define __TRAFFIC_H__

#include <pthread.h>

#define LANE_LENGTH 10

/* directions */
enum direction {
    NORTH,
    WEST,
    SOUTH,
    EAST,
    MAX_DIRECTION
};

/* representation of a car entering the intersection */
struct car {
    /* id that uniquely identifies each car */
    int             id;

    /* direction from which the car is entering and leaving */
    enum direction  in_dir, out_dir;

    /* support for singly linked list */
    struct car      *next;
};

/* entry lane feeding into the intersection */
struct lane {
    /* synchronization */
    pthread_mutex_t lock;
    pthread_cond_t  producer_cv, consumer_cv;

    /* list of cars that are pending to pass through this lane */
    struct car      *in_cars;

    /*
     * list of cars that have passed the intersection into this lane.
     * This list should only be appended to and left in memory for us
     * to use during testing. Please do not free the list or any of the cars
     */
    struct car      *out_cars;

    /* number of cars passing through this lane */
    int             inc;

    /* number of cars that have passed through this lane */
    int             passed;

    /* circular buffer implementation */
    struct car      **buffer;

    /* index of the first element element in the list */
    int             head;

    /* index of the last element in the list */
    int             tail;

    /* maximum number of elements in the list */
    int             capacity;

    /* number of elements currently in the list */
    int             in_buf;
};

/* complete representation of the intersection */
struct intersection {
    /* quadrants within the intersection
                   N
      +------------+------------+
      | Quadrant 2 | Quadrant 1 |
    W +------------+------------+ E
      | Quadrant 3 | Quadrant 4 |
      +------------+------------+
                   S
    Locks are ordered in priority 1 > 2 > 3 > 4, to avoid deadlock
    */
    pthread_mutex_t quad[4];

    struct lane       lanes[4];
};

/* forward declaration of logic functions */
void parse_schedule(char *f_name);
void init_intersection();

void *car_arrive(void *arg);
void *car_cross(void *arg);
int *compute_path(enum direction in_dir, enum direction out_dir);

#endif
