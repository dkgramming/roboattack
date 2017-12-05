/**
 * Roboattack
 * CSCI-251
 * David Kisluk
 */

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * 2D Vector
 */
struct Vec2 {
    int x;
    int y;
};

/**
 * Are two position vectors adjacent to one another?
 * Returns true if x and y differ by at most 1.
 */
bool isAdjacent(struct Vec2 a, struct Vec2 b) {
    int xDiff = abs(a.x - b.y);
    int yDiff = abs(a.y - b.y);

    return (xDiff <= 1 && yDiff <= 1); 
}

int main() {
    MPI_Init(NULL, NULL);
    int rank;
    int world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);


    struct Vec2 target;
    target.x = 3; target.y = 3;

    struct Vec2 pos[6]; // disregard 0-th index
    pos[0].x = 3;
    pos[0].y = 4;

    pos[1].x = 6;
    pos[1].y = 1;

    pos[2].x = 5;
    pos[2].y = 7;

    pos[3].x = 0;
    pos[3].y = 6;

    pos[4].x = 2;
    pos[4].y = 0;

    printf("P%i: (%i, %i) Target: (%i, %i)\n", rank, pos[rank].x, pos[rank].y, target.x, target.y);

    int data_to_send = (int)isAdjacent(pos[rank], target);
    int send_count = 1;
    MPI_Datatype send_type = MPI_INT;
    int destination_ID = (rank + 1) % world;
    int send_tag = 42; // arbritrary currently
    int received_data;
    int receive_count = send_count;
    MPI_Datatype receive_type = send_type;
    int sender_ID = (rank - 1) % world;
    sender_ID = (sender_ID < 0) ? sender_ID + world : sender_ID; // modulus isn't perfect
    int receive_tag = send_tag;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Status status; // TODO: Figure this guy out

    MPI_Sendrecv(&data_to_send, send_count, send_type, destination_ID, send_tag,
      &received_data, receive_count, receive_type, sender_ID, receive_tag, 
      comm, &status);

    if (received_data == 1) {
        printf("P%i is our lord and savior!\n", sender_ID);
    } else {
        printf("P%i is a false prophet!\n", sender_ID);
    }

    MPI_Finalize();
}
