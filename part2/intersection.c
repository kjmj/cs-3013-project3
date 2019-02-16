#include "intersection.h"

int main(int argc, char *argv[]) {
    /*
     * Initialize the mutex locks for each queue
     */
    mutex_init();
    /*
     * initialize the car queues with each queue representing a line of waiting in each direction
     */

    queueInit(&car_q[NORTH], sizeof(car_t));
    queueInit(&car_q[WEST], sizeof(car_t));
    queueInit(&car_q[SOUTH], sizeof(car_t));
    queueInit(&car_q[EAST], sizeof(car_t));

    /*
     * Create Car Thread(s)
     */
    int i; //count the number of car threads created
    pthread_t carThreads[NUM_CARS];
    for (i = 0; i < NUM_CARS; ++i) {
        if (0 != pthread_create(&carThreads[i], NULL, start_car, (void *) (intptr_t) i)) {
            // if error creating threads
            fprintf(stderr, "Error: Cannot Create thread\n");
            exit(-1);
        }
    }

    /*
    * Join Car Thread(s)
    */
    for (i = 0; i < NUM_CARS; ++i) {
        if (0 != pthread_join(carThreads[i], NULL)) {
            // if error joining threads
            fprintf(stderr, "Error: Cannot Join Thread %d\n", i);
            exit(-1);
        }
    }

    //destroy each mutex for each queue
    destroy_mutex();

    return 0;
}

void *start_car(void *param) {
    // set up info for current car(thread)
    int car_id = (int) (intptr_t) param;
    car_t this_car;
    this_car.car_id = car_id;
    this_car.appr_dir = (car_direction_t) random() % DIRMAX; // generate random direction for this car
    this_car.turn = (car_turn_t) random() % TURNMAX; // generate random turn for this car

    //wait for random time before approaching intersection
    usleep((useconds_t) (random() % TIME_TO_APPROACH));

    // Enqueue this car
    pthread_mutex_lock(&q_mutex[this_car.appr_dir]);
    enqueue(&car_q[this_car.appr_dir], &this_car);
    pthread_mutex_unlock(&q_mutex[this_car.appr_dir]);

    while (1) {
        //storing the car and the end of queue
        pthread_mutex_lock(&q_mutex[this_car.appr_dir]);
        car_t endCar;
        queuePeek(&car_q[this_car.appr_dir], &endCar);
        pthread_mutex_unlock(&q_mutex[this_car.appr_dir]);

        if (this_car.car_id == endCar.car_id) {
            //dequeue and approach if this car at the end of queue
            pthread_mutex_lock(&q_mutex[this_car.appr_dir]);
            dequeue(&car_q[this_car.appr_dir], &endCar);
            pthread_mutex_unlock(&q_mutex[this_car.appr_dir]);
            printf("Car[%d] is approaching with approach direction %s and turn %s \n", this_car.car_id,
                   i2s(this_car.appr_dir),
                   i2t(this_car.turn));
            break;

        }
    }
    /*
     * Intersection Logic
     */
    int canExit = 0;
    while (!canExit) {
        switch (this_car.appr_dir) {
            case NORTH: // North
                switch (this_car.turn) {
                    case RIGHT: // West right turn
                        if (pthread_mutex_lock(&northWest) == 0) {
                            canExit = 1;
                            printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                   i2s(this_car.appr_dir),
                                   i2t(this_car.turn));
                            //wait
                            usleep(TIME_TO_CROSS);
                            //release mutexes in order
                            pthread_mutex_unlock(&northWest);
                        }
                        break;
                    case STRAIGHT: // South straight
                        if (pthread_mutex_lock(&northWest) == 0) {
                            if (pthread_mutex_lock(&southWest) == 0) {
                                canExit = 1;
                                printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                       i2s(this_car.appr_dir),
                                       i2t(this_car.turn));
                                //wait
                                usleep(2 * TIME_TO_CROSS);
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                                pthread_mutex_unlock(&southWest);
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                            }
                        }
                        break;
                    case LEFT: // East left turn
                        if (pthread_mutex_lock(&northWest) == 0) {
                            if (pthread_mutex_lock(&southWest) == 0) {
                                if (pthread_mutex_lock(&southEast) == 0) {
                                    canExit = 1;
                                    printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                           i2s(this_car.appr_dir),
                                           i2t(this_car.turn));
                                    //wait
                                    usleep(3 * TIME_TO_CROSS);
                                    //release mutexes in order
                                    pthread_mutex_unlock(&northWest);
                                    pthread_mutex_unlock(&southWest);
                                    pthread_mutex_unlock(&southEast);
                                } else {
                                    //not all locks are acquired
                                    pthread_mutex_unlock(&southWest);
                                    pthread_mutex_unlock(&northWest);
                                }
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                            }
                        }
                        break;
                    default:
                        printf("THIS SHOULD NEVER HAPPEN unless the direction was set to something it shouldnt have been.\n");
                        break;
                }
                break;

            case WEST: // West
                switch (this_car.turn) {
                    case RIGHT: //right to south
                        if (pthread_mutex_lock(&southWest) == 0) {
                            canExit = 1;
                            printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                   i2s(this_car.appr_dir),
                                   i2t(this_car.turn));
                            //wait
                            usleep(TIME_TO_CROSS);
                            //release mutexes in order
                            pthread_mutex_unlock(&southWest);
                        }
                        break;
                    case STRAIGHT: // East straight
                        if (pthread_mutex_lock(&southWest) == 0) {
                            if (pthread_mutex_lock(&southEast) == 0) {
                                canExit = 1;
                                printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                       i2s(this_car.appr_dir),
                                       i2t(this_car.turn));
                                //wait
                                usleep(2 * TIME_TO_CROSS);
                                //not all locks are acquired
                                pthread_mutex_unlock(&southWest);
                                pthread_mutex_unlock(&southEast);
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&southWest);
                            }
                        }
                        break;
                    case LEFT: // North left turn
                        if (pthread_mutex_lock(&southWest) == 0) {
                            if (pthread_mutex_lock(&southEast) == 0) {
                                if (pthread_mutex_lock(&northEast) == 0) {
                                    canExit = 1;
                                    printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                           i2s(this_car.appr_dir),
                                           i2t(this_car.turn));
                                    //wait
                                    usleep(3 * TIME_TO_CROSS);
                                    //release mutexes in order
                                    pthread_mutex_unlock(&southWest);
                                    pthread_mutex_unlock(&southEast);
                                    pthread_mutex_unlock(&northEast);
                                } else {
                                    //not all locks are acquired
                                    pthread_mutex_unlock(&southWest);
                                    pthread_mutex_unlock(&southEast);
                                }
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&southWest);
                            }
                        }
                        break;
                    default:
                        printf("THIS SHOULD NEVER HAPPEN unless the direction was set to something it shouldnt have been.\n");
                        break;
                }
                break;

            case SOUTH: // South
                switch (this_car.turn) {
                    case RIGHT: //right to East
                        if (pthread_mutex_lock(&southEast) == 0) {
                            canExit = 1;
                            printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                   i2s(this_car.appr_dir),
                                   i2t(this_car.turn));
                            //wait
                            usleep(TIME_TO_CROSS);
                            //release mutexes in order
                            pthread_mutex_unlock(&southEast);
                        }
                        break;
                    case STRAIGHT: // North straight
                        if (pthread_mutex_lock(&southEast) == 0) {
                            if (pthread_mutex_lock(&northEast) == 0) {
                                canExit = 1;
                                printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                       i2s(this_car.appr_dir),
                                       i2t(this_car.turn));
                                //wait
                                usleep(2 * TIME_TO_CROSS);
                                //not all locks are acquired
                                pthread_mutex_unlock(&southEast);
                                pthread_mutex_unlock(&northEast);
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&southEast);
                            }
                        }
                        break;
                    case LEFT: // West left turn
                        if (pthread_mutex_lock(&northWest) == 0) {
                            if (pthread_mutex_lock(&southEast) == 0) {
                                if (pthread_mutex_lock(&northEast) == 0) {
                                    canExit = 1;
                                    printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                           i2s(this_car.appr_dir),
                                           i2t(this_car.turn));
                                    //wait
                                    usleep(3 * TIME_TO_CROSS);
                                    //release mutexes in order
                                    pthread_mutex_unlock(&northWest);
                                    pthread_mutex_unlock(&southEast);
                                    pthread_mutex_unlock(&northEast);
                                } else {
                                    //not all locks are acquired
                                    pthread_mutex_unlock(&northWest);
                                    pthread_mutex_unlock(&southEast);
                                }
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                            }
                        }
                        break;
                    default:
                        printf("THIS SHOULD NEVER HAPPEN unless the direction was set to something it shouldnt have been.\n");
                        break;
                }
                break;

            case EAST: // East
                switch (this_car.turn) {
                    case RIGHT: //right to North
                        if (pthread_mutex_lock(&northEast) == 0) {
                            canExit = 1;
                            printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                   i2s(this_car.appr_dir),
                                   i2t(this_car.turn));
                            //wait
                            usleep(TIME_TO_CROSS);
                            //release mutexes in order
                            pthread_mutex_unlock(&northEast);
                        }
                        break;
                    case STRAIGHT: // West straight
                        if (pthread_mutex_lock(&northWest) == 0) {
                            if (pthread_mutex_lock(&northEast) == 0) {
                                canExit = 1;
                                printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                       i2s(this_car.appr_dir),
                                       i2t(this_car.turn));
                                //wait
                                usleep(2 * TIME_TO_CROSS);
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                                pthread_mutex_unlock(&northEast);
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                            }
                        }
                        break;
                    case LEFT: // South left turn
                        if (pthread_mutex_lock(&northWest) == 0) {
                            if (pthread_mutex_lock(&southWest) == 0) {
                                if (pthread_mutex_lock(&northEast) == 0) {
                                    canExit = 1;
                                    printf("Car[%d] is entering from direction %s and turning %s \n", this_car.car_id,
                                           i2s(this_car.appr_dir),
                                           i2t(this_car.turn));
                                    //wait
                                    usleep(3 * TIME_TO_CROSS);
                                    //release mutexes in order
                                    pthread_mutex_unlock(&northWest);
                                    pthread_mutex_unlock(&southWest);
                                    pthread_mutex_unlock(&northEast);
                                } else {
                                    //not all locks are acquired
                                    pthread_mutex_unlock(&northWest);
                                    pthread_mutex_unlock(&southWest);
                                }
                            } else {
                                //not all locks are acquired
                                pthread_mutex_unlock(&northWest);
                            }
                        }
                        break;
                    default:
                        printf("THIS SHOULD NEVER HAPPEN unless the direction was set to something it shouldnt have been.\n");
                        break;
                }
                break;
            default:
                printf("THIS SHOULD NEVER HAPPEN");
                break;
        }
        printf("Car[%d] is leaving the intersection\n", this_car.car_id);
    }
    pthread_exit((void *) 0);
    return NULL;
}

void mutex_init() {
    for (int j = 0; j < DIRMAX; j++) {
        if (pthread_mutex_init(&q_mutex[j], NULL) != 0) {
            printf("\n mutex init[%d] has failed\n", j);
            exit(-1);
        }
    }
    if (pthread_mutex_init(&northWest, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(-1);
    }
    if (pthread_mutex_init(&northEast, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(-1);
    }
    if (pthread_mutex_init(&southEast, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(-1);
    }
    if (pthread_mutex_init(&southWest, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(-1);
    }
}

void destroy_mutex() {
    for (int j = 0; j < DIRMAX; j++) {
        if (pthread_mutex_destroy(&q_mutex[j]) != 0) {
            printf("\n mutex destroy[%d] has failed\n", j);
            exit(-1);
        }
    }
    if (pthread_mutex_destroy(&northWest) != 0) {
        printf("\n mutex destroy has failed\n");
        exit(-1);
    }
    if (pthread_mutex_destroy(&northEast) != 0) {
        printf("\n mutex destroy has failed\n");
        exit(-1);
    }
    if (pthread_mutex_destroy(&southWest) != 0) {
        printf("\n mutex destroy has failed\n");
        exit(-1);
    }
    if (pthread_mutex_destroy(&southEast) != 0) {
        printf("\n mutex destroy has failed\n");
        exit(-1);
    }
}

char *i2s(car_direction_t direction) {
    switch (direction) {
        case NORTH:
            return "North";
        case WEST:
            return "West";
        case SOUTH:
            return "South";
        case EAST:
            return "East";
        default:
            printf("this i2s function shouldn't happen");
            break;
    }
    return NULL;
}

char *i2t(car_turn_t turn) {
    switch (turn) {
        case LEFT:
            return "Left";
        case RIGHT:
            return "Right";
        case STRAIGHT:
            return "Straight";
        default:
            printf("this i2s function shouldn't happen");
            break;
    }
    return NULL;
}
