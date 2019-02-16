#include "part1.h"

#define EMPTY 0
#define PIRATE 1
#define NINJA 2

#define MAX_CUSTOMERS 100
#define MAX_TEAMS 4

pthread_t customerThreads[MAX_CUSTOMERS];

// for tracking the avg queue length statistics
pthread_t queueLengthChecker;
float totalInQueue = 0;
float numTimesChecked = 0;

// for storing customers
struct customer customers[MAX_CUSTOMERS];

// this is our room that pirates and ninjas go to get costumed
struct costumeDept room;

// number of pirates and ninjas in the simulation
int numPirates;
int numNinjas;


int main(int argc, char **argv) {
    if(argc != 8) {
        printf("Incorrect number of arguments. Usage: ./part1 numCostumingTeams numPirates numNinjas avgPirateCostumingTime avgNinjaCostumingTime avgPirateArrivalTime avgNinjaArrivalTime\n");
        return 1;
    }

    // read in command line arguments
    int numCostumingTeams = 4;//atoi(argv[1]);
    numPirates = atoi(argv[2]);
    numNinjas = atoi(argv[3]);
    int avgPirateCostumingTime = atoi(argv[4]);
    int avgNinjaCostumingTime = atoi(argv[5]);
    int avgPirateArrivalTime = atoi(argv[6]);
    int avgNinjaArrivalTime = atoi(argv[7]);

    // make sure command line args fall within specified bounds
    if(numCostumingTeams < MIN_COSTUME_TEAMS || numCostumingTeams > MAX_COSTUME_TEAMS) {
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
    room.teamBusyTime = 0;
    room.teamFreeTime = 0;

    // seed drand48()
    srand48(getCurrTimeInSeconds());

    pthread_create(&queueLengthChecker, NULL, (void *) &queueChecker, NULL);
    // create threads for pirate and ninja
    for (int i = 0; i < (numPirates + numNinjas); i++) {
        if (i < numPirates) { // pirates
            customers[i].type = PIRATE;
            strcpy(customers[i].typeStr, "Pirate");
            customers[i].arrivalTime = avgPirateArrivalTime * getRand();
            customers[i].costumingTime = avgPirateCostumingTime * getRand();
            customers[i].numVisits = 0;
            customers[i].totalGoldOwed = 0;
        }
        else { // ninjas
            customers[i].type = NINJA;
            strcpy(customers[i].typeStr, "Ninja");
            customers[i].arrivalTime = avgNinjaArrivalTime * getRand();
            customers[i].costumingTime = avgNinjaCostumingTime * getRand();
            customers[i].numVisits = 0;
            customers[i].totalGoldOwed = 0;
        }
        pthread_create(&customerThreads[i], NULL, (void *) &customer, (void *) i);
    }

    // Join threads
    for (int i = 0; i < (numPirates + numNinjas); i++) {
        pthread_join(customerThreads[i], NULL);
    }

    pthread_join(queueLengthChecker, NULL);

    // print the stats
    printStats();

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
    double startTime = 0; // time when the customer arrives at the shop
    int p = 0; // counter for the loop

    sem_post(&room.frontDoor);

    while (!done) {
        // wait for a customer to arrive
        sleep(customers[custNum].arrivalTime);

        // update their stats
        customers[custNum].numVisits++;
        startTime = getCurrTimeInSeconds();

        // serve the customer
        serveCustomer(customers[custNum].type, custNum, p);

        sem_wait(&room.frontDoor);
        // update their stats after the visit
        customers[custNum].visits[p].visitNum = customers[custNum].numVisits;
        customers[custNum].visits[p].visitTime = getCurrTimeInSeconds() - startTime;

        // decide if this customer will return
        if(drand48() <= .25 && customers[custNum].numVisits == 1) {
            printf("Returning soon: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
            p++;
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
void serveCustomer(int type, int custNum, int visitNum) {
    sem_wait(&room.frontDoor);

    printf("Arrived: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
    double arrivalTime = getCurrTimeInSeconds();

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

        int freeTimeStart = getCurrTimeInSeconds();

        room.numCustomers++;
        room.waitingPirates--;
        printf("Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);

        // update the customers stats
        customers[custNum].visits[visitNum].waitTime = getCurrTimeInSeconds() - arrivalTime;

        // a team is busy
        room.teamFreeTime = room.teamFreeTime + getCurrTimeInSeconds() - freeTimeStart;
        int busyTimeStart = getCurrTimeInSeconds();

        sem_post(&room.frontDoor);

        // costume this customer
        sleep(customers[custNum].costumingTime);

        sem_wait(&room.frontDoor);

        printf("Done Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
        room.numCustomers--;

        // update the customers stats
        customers[custNum].visits[visitNum].goldOwed = customers[custNum].visits[visitNum].waitTime + customers[custNum].costumingTime;
        customers[custNum].totalGoldOwed = customers[custNum].totalGoldOwed + customers[custNum].visits[visitNum].goldOwed;

        // the team is no longer busy
        room.teamBusyTime = room.teamBusyTime + getCurrTimeInSeconds() - busyTimeStart;

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
        int freeTimeStart = getCurrTimeInSeconds();

        room.numCustomers++;
        room.waitingNinjas--;
        printf("Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);

        // update the customers stats
        customers[custNum].visits[visitNum].waitTime = getCurrTimeInSeconds() - arrivalTime;

        // a team is now busy
        room.teamFreeTime = room.teamFreeTime + getCurrTimeInSeconds() - freeTimeStart;
        int busyTimeStart = getCurrTimeInSeconds();

        sem_post(&room.frontDoor);

        // costume this customer
        sleep(customers[custNum].costumingTime);

        sem_wait(&room.frontDoor);

        printf("Done Costuming: Customer #%d (%s)\n", custNum, customers[custNum].typeStr);
        room.numCustomers--;

        // update the customers stats
        customers[custNum].visits[visitNum].goldOwed = customers[custNum].visits[visitNum].waitTime + customers[custNum].costumingTime;
        customers[custNum].totalGoldOwed = customers[custNum].totalGoldOwed + customers[custNum].visits[visitNum].goldOwed;

        // the team is no longer busy
        room.teamBusyTime = room.teamBusyTime + getCurrTimeInSeconds() - busyTimeStart;

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
 * check the queue every second and gather stats about the average number of customers
 * in the queue
 */
void queueChecker() {
    sleep(1);

    sem_wait(&room.frontDoor);
    totalInQueue = totalInQueue + (room.waitingNinjas + room.waitingPirates);
    numTimesChecked++;
    sem_post(&room.frontDoor);
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
double getCurrTimeInSeconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (1.0/1000000) * t.tv_usec;
}

/**
 * print stats about the costume shop after everyone has been served
 */
void printStats() {

    float grossRevenue = 0;
    int numVisits = 0;

    // print stats for each customer
    printf("\n===================\n");
    printf("Customer Statistics\n");
    printf("===================\n\n");

    for(int i = 0; i < (numPirates + numNinjas); i++) {
        printf("Customer #%d (%s) Number of Visits: %d\n", i, customers[i].typeStr, customers[i].numVisits);
        for(int j = 0; j < customers[i].numVisits; j++) {
            printf("Customer #%d (%s) Visit Number: %d\n", i, customers[i].typeStr, j + 1);
            printf("Customer #%d (%s) Visit Length: %f\n", i, customers[i].typeStr, customers[i].visits[j].visitTime);
            printf("Customer #%d (%s) Wait Time: %f\n", i, customers[i].typeStr, customers[i].visits[j].waitTime);
            numVisits++;
        }
        printf("Customer #%d (%s) Total Gold Owed: %f\n", i, customers[i].typeStr, customers[i].totalGoldOwed);
        grossRevenue = grossRevenue + customers[i].totalGoldOwed;

        // formatting
        if(i < (numPirates + numNinjas) - 1) {
            printf("===================\n");
        }
    }

    // print shop stats
    printf("\n===================\n");
    printf("  Shop Statistics\n");
    printf("===================\n\n");

    printf("Time that teams were busy: %f\n", room.teamBusyTime);
    printf("Time that teams were free: %f\n", room.teamFreeTime);
    printf("Average Queue Length: %f\n", totalInQueue / numTimesChecked);
    printf("Gross Revenue: %f\n", grossRevenue);
    printf("Gold Per Visit: %f\n", grossRevenue / numVisits);
    printf("Total Profit: %f\n", grossRevenue - (room.numTeams * 5)); // it costs 5 gold per day to staff a team

}