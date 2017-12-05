/**
 * Roboattack
 * CSCI-251
 * David Kisluk
 */

#include <mpi.h>
#include <stdio.h>

struct Vec2 {
    int x;
    int y;
};

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

    MPI_Finalize();
}
