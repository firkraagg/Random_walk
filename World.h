#ifndef WORLD_H
#define WORLD_H

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "Pedestrian.h"

typedef struct Pedestrian Pedestrian;
typedef enum SimulationMode SimulationMode;

typedef enum WorldType {
    WORLD_EMPTY,
    WORLD_OBSTACLES_GENERATED,
    WORLD_OBSTACLES_FILE
} WorldType;

typedef struct World {
    int midX_;
    int midY_;
    int width_;
    int height_;
    char** grid_;
    char** initial_grid_;
    int** stepsGrid_;
    double probabilities_[4];
    double** probabilityGrid_;
    char* inputFileName_;
    char* outputFileName_;
    WorldType worldType_;
    Pedestrian* pedestrian_;
    Pedestrian* initial_pedestrian_;
} World;

void initialize_world(World* world, SimulationMode mode, WorldType worldType, int width, int height, int K, float probabilities[4]);
void initialize_probabilities(float probabilities[4], double probabilities_double[4]);
void allocate_grid(World* world, int height, int width);
void initialize_grid(World* world, SimulationMode mode, WorldType worldType, int height, int width, int K);
void initialize_steps_grid(World* world, int height, int width);
void initialize_probability_grid(World* world, int height, int width, int K);
void save_initial_state(World* world);
void reset_world(World* world);
void send_world(int client_socket, World* world);
void print_world(World* world);
void print_world_summary(World* world, SimulationMode mode);
void reinitialize_world_pedestrian(World* world);
int read_world_from_file(World* world);
const char* world_type_to_string(WorldType worldType);
void calculate_center(World* world);
double calculate_expected_steps(int x, int y, int midX, int midY, double probabilities[4]);
double calculate_probability_to_center(int x, int y, int midX, int midY, int K, double probabilities[4]);
void generate_world(World* world);
void free_world(World* world);

#endif //WORLD_H