/**
 * Roboattack
 * CSCI-251
 * David Kisluk
 */

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#define DBL_EPSILON 2.2204460492503131e-16

const int NOT_ZERO = 42;

const int WIDTH = 30;
const int HEIGHT = 50;

const int X = 0;
const int Y = 1;
const int UNKNOWN = -1;
#define TUPLE 2
#define TRIPLE 3

struct Message {
    int rank;
    int distance;
    int tag;
};

const int ELECTION_TAG = 0;
const int LEADER_TAG = 1;

int manhattan_distance(int x1, int y1, int x2, int y2) {
    return (abs(x2-x1) + abs(y2-y1));
}

double euclidian_distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}

bool fequal(double a, double b) {
    return fabs(a-b) < DBL_EPSILON;
}

int main() {
    /* Initialize MPI */
    MPI_Init(NULL, NULL);
    int rank;
    int world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    /* Seed the PRNG */
    time_t t;
    srand((unsigned)time(&t) * rank);

    /* Initialize the robot's view of the world */
    int grid[HEIGHT][WIDTH];

    /* Target location is not known */
    int target[TUPLE];
    target[X] = UNKNOWN;
    target[Y] = UNKNOWN;

    /* but technically some process has to know it */
    int actual_target[TUPLE];
    actual_target[X] = 5;
    actual_target[Y] = 5;

    /* Randomize robot starting location */
    int pos[world][TUPLE];
    pos[rank][X] = rand() % HEIGHT;
    pos[rank][Y] = rand() % WIDTH;

    printf("P%i: (%i, %i) Target: (%i, %i)\n", 
        rank, pos[rank][X], pos[rank][Y], target[X], target[Y]);

    struct Message data_to_send;
    data_to_send.rank = rank;
    data_to_send.distance = manhattan_distance(pos[rank][X], pos[rank][Y], target[X], target[Y]);
    data_to_send.tag = ELECTION_TAG;

    int destination_ID = (rank + 1) % world;
    struct Message received_data;
    int sender_ID = (rank - 1) % world;
    sender_ID = (sender_ID < 0) ? sender_ID + world : sender_ID; // modulus isn't perfect
    MPI_Status status; // TODO: Figure this guy out

    /** 
     * Begin leader election 
     */
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
        if (received_data.tag == ELECTION_TAG) {
            printf("P%i votes for P%i\n", sender_ID, received_data.rank);

            if (received_data.rank > rank) {
                /* Vote for the received process */
                data_to_send.rank = received_data.rank;
            }
            if (rank > received_data.rank) {
                /* Vote for self */
                data_to_send.rank = rank;
            }
            if (rank == received_data.rank) {
                /* Declare self the leader */
                data_to_send.rank = rank;
                data_to_send.tag = LEADER_TAG;
                printf("P%i has been elected\n", rank);
            }

            MPI_Send(&data_to_send, TRIPLE, MPI_INT, destination_ID, ELECTION_TAG, MPI_COMM_WORLD);
        }
        /* Receive leader message */
        if (received_data.tag == LEADER_TAG) {
            if (received_data.rank != rank) {
                data_to_send.rank = received_data.rank;
                data_to_send.tag = true;
                printf("P%i heard that P%i has been elected\n", rank, data_to_send.rank);
            }
            MPI_Send(&data_to_send, TRIPLE, MPI_INT, destination_ID, ELECTION_TAG, MPI_COMM_WORLD);
            leader_rank = received_data.rank;
        }
    }

    /**
     * Search for the target
     */

    /**
     * Coordinate the target surround process
     */
    int destinations[5*TUPLE]; // TODO: Replace magic number 
    int destination[TUPLE];
    if (rank == leader_rank) {
        /* Allow the elected leader to decide each robot's destination cell */
        int num_assignments= 0;
        double distance = 1.0;
        int a = 1;
        int b = 0;
        bool is_assigned[5];
        while (num_assignments < world) {
            distance = euclidian_distance(0, 0, a, b);

            for (int x_offset = -distance; x_offset <= distance && num_assignments < world; x_offset++) {
                for (int y_offset = -distance; y_offset <= distance && num_assignments < world; y_offset++) {
                    int x_pos = target[X] + x_offset;
                    int y_pos = target[Y] + y_offset;
                    bool close_enough = fequal(distance, euclidian_distance(x_pos, y_pos, target[X], target[Y]));
                    bool within_grid = x_pos >= 0 && x_pos < HEIGHT && y_pos >= 0 && y_pos < WIDTH;
                    if (close_enough && within_grid) {
                        /* Pick the closest unassigned robot */
                        int closest_pid = -1;
                        int min_dist = INT_MAX;
                        for (int pid = 0; pid < world; pid++) {
                            int distance = manhattan_distance(pos[pid][X], pos[pid][Y], x_pos, y_pos);
                            if (!is_assigned[pid] && distance < min_dist) {
                                min_dist = distance;
                                closest_pid = pid;
                            }
                        }

                        printf("P%i has been assigned to (%i, %i)\n", closest_pid, x_pos, y_pos);
                        destinations[2*closest_pid] = x_pos;
                        destinations[2*closest_pid+1] = y_pos;
                        num_assignments++;
                        is_assigned[closest_pid] = true;
                    }
                }
            }
            /* Extend search to next "ring" around the target */
            if (a > b)
                b++;
            else {
                a++;
                b = 0;
            }
        }
    } 
    /* All other robots await the leader's broadcast */ 
    MPI_Scatter(&destinations, TUPLE, MPI_INT, &destination, TUPLE, MPI_INT, leader_rank, MPI_COMM_WORLD);
    printf("P%i instructed P%i to move to (%i, %i)\n", leader_rank, rank, destination[X], destination[Y]);

    /**
     * Move to the destination
     */
    int diff = manhattan_distance(pos[rank][X], pos[rank][Y], destination[X], destination[Y]);
    int total_diff = NOT_ZERO; 
    int future_pos[5*TUPLE];
    int my_future_pos[TUPLE];
    
    while (total_diff != 0) {
        /* Sum all robots' distance from their destination */
        MPI_Allreduce(&diff, &total_diff, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        /* By default, no movement */
        future_pos[2*rank] = pos[rank][X];
        future_pos[2*rank+1] = pos[rank][Y];

        /* Move towards the destination if not there already */
        if (diff != 0) {
            if (pos[rank][X] < destination[X]) { // TODO: toggle x and y
                future_pos[2*rank] = pos[rank][X] + 1; // DOWN
            } else if (pos[rank][Y] < destination[Y]) {
                future_pos[2*rank+1] = pos[rank][Y] + 1; // RIGHT
            } else if (pos[rank][X] > destination[X]) {
                future_pos[2*rank] = pos[rank][X] - 1; // UP
            } else if (pos[rank][Y] > destination[Y]) {
                future_pos[2*rank+1] = pos[rank][Y] - 1; // LEFT
            }  
        }

        /* Detect collisions in future position */
        my_future_pos[X] = future_pos[2*rank];
        my_future_pos[Y] = future_pos[2*rank+1];
        MPI_Allgather(&my_future_pos, TUPLE, MPI_INT, &future_pos, TUPLE, MPI_INT, MPI_COMM_WORLD);
        
        for (int other = 0; other < world; other++) {
            if (rank < other &&                                 // only consider higher ranks
                future_pos[2*rank] == future_pos[2*other] &&    // same X
                future_pos[2*rank+1] == future_pos[2*other+1]) {    // same Y
                
                /* Collision detected */
                printf("!!!P%i averted a collision with P%i!!!\n", rank, other);   
                future_pos[2*rank] = pos[rank][X];
                future_pos[2*rank+1] = pos[rank][Y];
                break;  
            }
        }

        /* Commit the movement */
        pos[rank][X] = future_pos[2*rank];
        pos[rank][Y] = future_pos[2*rank+1];
            
        diff = manhattan_distance(pos[rank][X], pos[rank][Y], destination[X], destination[Y]);
        printf("P%i is %i cells from its destination\n", rank, diff);
    }

    MPI_Finalize();
}
