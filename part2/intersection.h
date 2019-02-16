#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"

#define NUM_CARS 20
#define NUM_INTER 4
#define TIME_TO_CROSS 1000
#define TIME_TO_APPROACH 50000

queue car_q[NUM_INTER];
pthread_mutex_t q_mutex[NUM_INTER];
pthread_mutex_t northWest;
pthread_mutex_t northEast;
pthread_mutex_t southEast;
pthread_mutex_t southWest;

/* encode the direction of a car */
typedef enum {
    NORTH = 0,
    WEST = 1,
    SOUTH = 2,
    EAST = 3,
    DIRMAX = 4
} car_direction_t;

/* encode the turning of the car */
typedef enum {
    RIGHT = 0,
    STRAIGHT = 1,
    LEFT = 2,
    TURNMAX = 3
} car_turn_t;

/* A general car structure to hold all of the information */
struct car_t {
    int car_id;
    car_direction_t appr_dir;
    car_turn_t turn;
};
typedef struct car_t car_t;

/* function called by car thread*/
void *start_car(void *param);

void mutex_init();

void destroy_mutex();

char *i2s(car_direction_t turn);

char *i2t(car_turn_t turn);

