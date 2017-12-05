/**
 * Roboattack
 * CSCI-251
 * David Kisluk
 */

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

const int WIDTH = 10;
const int HEIGHT = 10;

const int RANK = 0;
const int DISTANCE = 1;
const int TAG = 2;

const int X = 0;
const int Y = 1;
#define TUPLE 2
#define TRIPLE 3

const int ELECTION_TAG = 0;
const int LEADER_TAG = 1;

int manhattan_distance(int x1, int x2, int y1, int y2) {
    return (abs(x2-x1) + abs(y2-y1));
}

int main() {
    /* Initialize MPI */
    MPI_Init(NULL, NULL);
    int rank;
    int world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    /* Initialize the robot's view of the world */
    int grid[HEIGHT][WIDTH];

    /* Pretend the target location and robot locations are known */
    int target[TUPLE];
    target[X] = 3; target[Y] = 3;
    int pos[world][TUPLE];
    pos[0][X] = 3;
    pos[0][Y] = 4;

    pos[1][X] = 6;
    pos[1][Y] = 1;

    pos[2][X] = 5;
    pos[2][Y] = 7;

    pos[3][X] = 0;
    pos[3][Y] = 6;

    pos[4][X] = 2;
    pos[4][Y] = 0;

    printf("P%i: (%i, %i) Target: (%i, %i)\n", 
        rank, pos[rank][X], pos[rank][Y], target[X], target[Y]);

    int data_to_send[TRIPLE] = { 
        rank, 
        manhattan_distance(pos[rank][X], pos[rank][Y], target[X], target[Y]),
        ELECTION_TAG
    };
    int destination_ID = (rank + 1) % world;
    int received_data[TUPLE];
    int sender_ID = (rank - 1) % world;
    sender_ID = (sender_ID < 0) ? sender_ID + world : sender_ID; // modulus isn't perfect
    MPI_Status status; // TODO: Figure this guy out

    /* Begin leader election */
    int leader_rank = -1; // no leader
    int my_distance = manhattan_distance(pos[rank][X], pos[rank][Y], target[X], target[Y]); 

    /* Process 0 gets the honor of starting the election */
    if (rank == 0) {
        MPI_Send(&data_to_send, TRIPLE, MPI_INT, destination_ID, ELECTION_TAG, MPI_COMM_WORLD);
    }

    /* Continue receiving messages until leader is chosen */
    while (leader_rank == -1) {
        MPI_Recv(&received_data, TRIPLE, MPI_INT, sender_ID, ELECTION_TAG,
            MPI_COMM_WORLD, &status);


        /* Receive election message */
        if (received_data[TAG] == ELECTION_TAG) {
            printf("P%i votes for P%i\n", sender_ID, received_data[0]);

            if (received_data[0] > rank) {
                // election(received_data[0]);
                data_to_send[0] = received_data[0];
            }
            if (rank > received_data[0]) {
                // election(rank);
                data_to_send[0] = rank;
            }
            if (rank == received_data[0]) {
                data_to_send[0] = rank;
                data_to_send[2] = true;
                printf("P%i has been elected\n", rank);
                // leader(rank);
            }
            MPI_Send(&data_to_send, TRIPLE, MPI_INT, destination_ID, ELECTION_TAG, MPI_COMM_WORLD);
        }
        /* Receive leader message */
        if (received_data[TAG] == LEADER_TAG) {
            if (received_data[0] != rank) {
                data_to_send[0] = received_data[0];
                data_to_send[2] = true;
                printf("P%i heard that P%i has been elected\n", rank, data_to_send[0]);
            }
            MPI_Send(&data_to_send, TRIPLE, MPI_INT, destination_ID, ELECTION_TAG, MPI_COMM_WORLD);
            leader_rank = received_data[RANK];
        }
    }

    MPI_Finalize();
}
