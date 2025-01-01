#ifndef WORLD_H
#define WORLD_H

#include <stdlib.h>
#include <stdio.h>
#include "Pedestrian.h"

typedef struct Pedestrian Pedestrian;
typedef enum SimulationMode SimulationMode;

typedef enum WorldType {
    WORLD_EMPTY,
    WORLD_OBSTACLES_FILE,
    WORLD_OBSTACLES_GENERATED
} WorldType;

typedef struct World {
    int midX_;
    int midY_;
    int width_;
    int height_;
    char** grid_;
    char* inputFileName_;
    WorldType worldType_;
    Pedestrian* pedestrian_;
} World;

void initialize_world(World* world, Pedestrian* pedestrian, SimulationMode mode, WorldType worldType, int width, int height, int K);
void print_world(World* world);
int read_world_from_file(World* world);
void calculate_center(World* world);
double calculate_expected_steps(int x, int y, int midX, int midY, double probabilities[4]);
//int calculate_probability_with_K_steps(int x, int y, int midX, int midY, int K);
void generate_world(World* world);
void free_world(World* world);

#endif //WORLD_H