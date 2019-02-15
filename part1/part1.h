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

/**
 * TODO we must collect these stats
 *
 * Each pirate and ninja must spend 1 gold piece for every minute inside of the costume department.
 * However, if any pirate or ninja has to wait for more than 30 minutes prior to entering the shop then you have to costume them for free.
 *
 * At the end of the day, you should calculate and print the itemized bills for both the pirates and the ninjas.
 * For each ninja/pirate you should list
 *   - the number of visits
 *   - the amount of time of each visit
 *   - the wait times
 *   - the total gold owed to the costume department.
 *
 * You also need to keep track of your own expenses profits.
 * Each costuming team costs 5 gold pieces per day to staff.
 * You should print also
 * - the amount of time that each team was busy
 * - the amount of time each team was free
 * - the average queue length
 * - the gross revenue (amount of gold)
 * - the gold-per-visit (amount of gold divided by number of visits)
 * - your total profits.
 */

#define MAX_VISITS 2

struct costumeTeam {
    int teamNum;
    float timeBusy; // amount of time this team was busy
    float timeFree; // amount of time this team was free
};

struct costumeDept {
    int currentlyServing; // ninjas or pirates
    int numTeams; // number of costuming teams
    int numCustomers; // number of customers currently in the shop
    int waitingPirates; // number of pirates waiting
    int waitingNinjas; // number of ninjas waiting
    sem_t frontDoor; // general lock for the front door
    sem_t ninjaDoor; // lock for the ninjas
    sem_t pirateDoor; // lock for the pirates

    // stat collection
    struct costumeTeam costumeTeam;
    float avgQueueLength;
    int grossRevenue;
    float goldPerVisit; // amount of gold divided by number of visits
    float totalProfits; // total profits for the costume dept
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
double getCurrTimeInSeconds();
void serveCustomer(int teamType, int custNum, int visitNum);
float getRand();
void printStats();

#endif