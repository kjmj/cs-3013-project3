----- Overview -----
This project simulates a dressing room that services pirates and ninjas. The pirates and ninjas cannot
be in the dressing room at the same time, however, it can serve multiple customers of the same type at the
same time. The pirates and ninjas each have their own door to the shop, so they will not see each other in that situation.

----- Usage -----
make all
./part1 numCostumingTeams numPirates numNinjas avgPirateCostumingTime avgNinjaCostumingTime avgPirateArrivalTime avgNinjaArrivalTime
Arguments
    numCostumingTeams: The number of costuming teams (min. 2, max. 4).
    numPirates: The number of pirates (10–50).
    numNinjas: The number of ninjas (10–50).
    avgPirateCostumingTime: The average costuming time of a pirate.
    avgNinjaCostumingTime: The average costuming time of a ninja.
    avgPirateArrivalTime: The average arrival time of a pirate.
    avgNinjaArrivalTime: The average arrival time of a ninja.

----- Testing -----
Please see the file 'problem1_explanation.txt' for multiple runs and analyses with varying input to the simulator