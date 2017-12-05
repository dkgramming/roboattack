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

const int X = 0;
const int Y = 1;
const int TUPLE = 2;

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

    printf("P%i: (%i, %i) Target: (%i, %i)\n", rank, pos[rank][X], pos[rank][Y], target[X], target[Y]);

    int data_to_send[2] = { false, rank };
    int send_count = 2;
    MPI_Datatype send_type = MPI_INT;
    int destination_ID = (rank + 1) % world;
    int send_tag = 42; // arbritrary currently
    int received_data[2];
    int receive_count = send_count;
    MPI_Datatype receive_type = send_type;
    int sender_ID = (rank - 1) % world;
    sender_ID = (sender_ID < 0) ? sender_ID + world : sender_ID; // modulus isn't perfect
    int receive_tag = send_tag;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Status status; // TODO: Figure this guy out

    MPI_Finalize();
}
