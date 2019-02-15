#include "part1.h"

#define EMPTY 0
#define PIRATE 1
#define NINJA 2

#define MAX_CUSTOMERS 100
#define MAX_TEAMS 4

pthread_t customerThreads[MAX_CUSTOMERS];

struct customer customers[MAX_CUSTOMERS];
struct costumeTeam costumeTeams[MAX_TEAMS];
struct costumeDept room;

int main(int argc, char **argv) {
    if(argc != 8) {
        printf("Incorrect number of arguments. Usage: ./part1 numCostumingTeams numPirates numNinjas avgPirateCostumingTime avgNinjaCostumingTime avgPirateArrivalTime avgNinjaArrivalTime\n");
        return 1;
    }

    // read in command line arguments
    int numCostumingTeams = atoi(argv[1]);
    int numPirates = atoi(argv[2]);
    int numNinjas = atoi(argv[3]);
    int avgPirateCostumingTime = atoi(argv[4]);
    int avgNinjaCostumingTime = atoi(argv[5]);
    int avgPirateArrivalTime = atoi(argv[6]);
    int avgNinjaArrivalTime = atoi(argv[7]);

    // make sure command line args fall within specified bounds
    if(numCostumingTeams < 2 || numCostumingTeams > 4) {
        printf("Please enter numCostumingTeams between [2, 4]\n");
        return 1;
    }
    if(numPirates < 10 || numPirates > 50) {
        printf("Please enter numPirates between [10, 50]\n");
        return 1;
    }
    if(numNinjas < 10 || numNinjas > 50) {
        printf("Please enter numPirates between [10, 50]\n");
        return 1;
    }

    // initialize our locks
    sem_init(&room.frontDoor, 0, 1);
    sem_init(&room.pirateDoor, 0, 0);
    sem_init(&room.ninjaDoor, 0, 0);

    // get the costuming room setup
    room.currentlyServing = EMPTY;
    room.numTeams = numCostumingTeams;
    room.numCustomers = 0;
    room.waitingPirates = 0;
    room.waitingNinjas = 0;

    // seed drand48()
    long t = getCurrTimeInSeconds();
    printf("random value: %ld\n", t);
    srand48(t);

    // setup costuming teams
    for (int i = 0; i < numCostumingTeams; i++) {
        costumeTeams[i].teamNum = i;
    }

    // create threads for pirate and ninja
    for (int i = 0; i < (numPirates + numNinjas); i++) {
        if (i < numPirates) { // pirates
            customers[i].type = PIRATE;
            strcpy(customers[i].typeStr, "Pirate");
            customers[i].arrivalTime = avgPirateArrivalTime * getRand();
            customers[i].costumingTime = avgPirateCostumingTime * getRand();
        }
        else { // ninjas
            customers[i].type = NINJA;
            strcpy(customers[i].typeStr, "Ninja");
            customers[i].arrivalTime = avgNinjaArrivalTime * getRand();
            customers[i].costumingTime = avgNinjaCostumingTime * getRand();
        }
        pthread_create(&customerThreads[i], NULL, (void *) &customer, (void *) i);
    }

    // Join threads
    for (int i = 0; i < (numPirates + numNinjas); i++) {
        pthread_join(customerThreads[i], NULL);
    }

    return 0;
}

/**
 * executes the behavior of a customer over a single day
 * @param custNumPtr
 */
void customer(void *custNumPtr) {

    sem_wait(&room.frontDoor);

    int custNum = (int) custNumPtr;
    int done = 0;
    int visitNum = 1;

    sem_post(&room.frontDoor);

    while (!done) {
        // wait for a customer to arrive
        sleep(customers[custNum].arrivalTime);

        // serve the customer
        serveCustomer(customers[custNum].type, custNum);

        sem_wait(&room.frontDoor);

        // decide if this customer will return
        if(drand48() <= .25 && visitNum == 1) {
            printf("Returning: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
            visitNum++;
        } else {
            done = 1;
        }

        sem_post(&room.frontDoor);
    }
}

/**
 * decide if we can serve this customer, and decide who to serve next
 * @param type
 * @param custNum
 */
void serveCustomer(int type, int custNum) {
    sem_wait(&room.frontDoor);

    printf("Arrived: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);

    // We now have one more waiting customer
    if(customers[custNum].type == PIRATE) {
        room.waitingPirates++;
    } else if(customers[custNum].type == NINJA) {
        room.waitingNinjas++;
    }

    // Make sure to account for the first customer
    if (room.currentlyServing == EMPTY) {
        if (type == PIRATE) {
            room.currentlyServing = PIRATE;
        } else if(type == NINJA) {
            room.currentlyServing = NINJA;
        }
    }

    sem_post(&room.frontDoor);


    if(type == PIRATE) {
        sem_wait(&room.frontDoor);

        // wait to serve this customer
        if(!(room.currentlyServing == type && room.numCustomers < room.numTeams)) {
            sem_post(&room.frontDoor);
            sem_wait(&room.pirateDoor);
            sem_wait(&room.frontDoor);
        }

        room.numCustomers++;
        room.waitingPirates--;
        printf("Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);

        sem_post(&room.frontDoor);

        // costume this customer
        sleep(customers[custNum].costumingTime);

        sem_wait(&room.frontDoor);

        printf("Done Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
        room.numCustomers--;

        // decide which customer to serve next
        if(room.numCustomers < room.numTeams) { // there must be room for the customer
            if(room.waitingNinjas > 0) { // if there is a waiting ninja and a free team, serve them
                if(room.numCustomers == 0) {
                    room.currentlyServing = NINJA;
                    sem_post(&room.ninjaDoor);
                }
            }
            else if (room.waitingPirates > 0) { // otherwise, serve a pirate
                sem_post(&room.pirateDoor);
            }
        }

        sem_post(&room.frontDoor);

    } else if(type == NINJA) {
        sem_wait(&room.frontDoor);

        // wait to serve this customer
        if(!(room.currentlyServing == type && room.numCustomers < room.numTeams)) {
            sem_post(&room.frontDoor);
            sem_wait(&room.ninjaDoor);
            sem_wait(&room.frontDoor);
        }

        room.numCustomers++;
        room.waitingNinjas--;
        printf("Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);

        sem_post(&room.frontDoor);

        // costume this customer
        sleep(customers[custNum].costumingTime);

        sem_wait(&room.frontDoor);

        printf("Done Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
        room.numCustomers--;

        // decide which customer to serve next
        if(room.numCustomers < room.numTeams) { // there must be room for the customer
            if(room.waitingPirates > 0) { // if there is a waiting pirate and a free team, serve them
                if(room.numCustomers == 0) {
                    room.currentlyServing = PIRATE;
                    sem_post(&room.pirateDoor);
                }
            }
            else if (room.waitingNinjas > 0) { // otherwise, serve a ninja
                sem_post(&room.ninjaDoor);
            }
        }

        sem_post(&room.frontDoor);
    }
}

/**
 * return a random number between 0 and 2
 * @return
 */
float getRand() {
    return 2 * drand48();
}

/**
 * Get the current time, in seconds
 * @return
 */
long getCurrTimeInSeconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec;
}
