#ifndef part1
#define part1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>

#define MAX_VISITS 2
#define MIN_COSTUME_TEAMS 2
#define MAX_COSTUME_TEAMS 4

struct costumeDept {
    int currentlyServing; // ninjas or pirates
    int numTeams; // number of costuming teams
    int numCustomers; // number of customers currently in the shop
    int waitingPirates; // number of pirates waiting
    int waitingNinjas; // number of ninjas waiting
    int servedSinceSwitch; // how many have we served since a switch between pirates and ninjas
    sem_t frontDoor; // general lock for the front door
    sem_t ninjaDoor; // lock for the ninjas
    sem_t pirateDoor; // lock for the pirates

    // stat collection
    float avgQueueLength;
    int grossRevenue;
    float goldPerVisit; // amount of gold divided by number of visits
    float totalProfits; // total profits for the costume dept
    double teamBusyTime; // how long were teams busy for
    double teamFreeTime; // how long were teams free for
};

struct visit {
    int visitNum;
    float visitTime; // amount of time for this visit
    float waitTime; // how long the customer waited
    float goldOwed; // how much gold the customer owes
};

struct customer {
    int type; // pirate or ninja
    char typeStr[10]; // pirate or ninja as a string
    float arrivalTime; // how long until the customer arrives
    float costumingTime; // how long does it take to costume this customer

    // stat collection
    struct visit visits[MAX_VISITS]; // max 2 visits for a customer
    int numVisits; // number of times the customer visited the shop
    float totalGoldOwed;
};

void customer();
void queueChecker();
double getCurrTimeInSeconds();
void serveCustomer(int teamType, int custNum, int visitNum);
float getRand();
void printStats();

#endif