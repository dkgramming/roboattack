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

    struct Vec2 position;
    position.x = rank;
    position.y = rank;

    printf("I'm at [%i, %i], look at me!\n", position.x, position.y);

    MPI_Finalize();
}
