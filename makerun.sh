#!/bin/bash
mpicc roboattack.c -lm -o roboattack && mpirun -np  5 ./roboattack
