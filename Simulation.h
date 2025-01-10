#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include "Pedestrian.h"

typedef struct World World;
typedef enum WorldType WorldType;

typedef enum SimulationMode {
    INTERACTIVE_MODE,
    SUMMARY_MODE_WITH_K,
    SUMMARY_MODE_WITHOUT_K
}   SimulationMode;

typedef struct SimulationInputs {
    bool singlePlayer;
    int numReplications;
    int K;
    float probabilities[4];
    char outputFileName[256];
    int worldType;
    int worldWidth;
    int worldHeight;
    char inputFileName[256];
    SimulationMode mode;
} SimulationInputs;

typedef struct Simulation {
    int numReplications_;
    float probabilities_[4];
    int K_;
    SimulationMode mode_;
    bool singlePlayer_;
    World* world_;
} Simulation;

void create_simulation(SimulationInputs* sp, Simulation* sim);
SimulationInputs* input_from_user();
void run_simulation(Simulation* simulation, int client_socket);
void finalize_simulation(Simulation* simulation, int client_socket);
void perform_replication(Simulation* simulation, int client_socket, size_t replication_index);
void initialize_simulation(Simulation* simulation);
void save_simulation_results(Simulation* simulation, const char* fileName);
void load_simulation_results(Simulation* simulation, const char* fileName);
Simulation* recreate_simulation();
void free_simulation(Simulation* simulation);

float* choose_probabilities();
int choose_number_of_replications();
int choose_world_type();
int choose_world_with_obstacles();
char* choose_input_file();
char* choose_output_file();
bool choose_player_mode();
int number_of_steps();
SimulationMode choose_mode();
void get_size(int* x, int* y);

#endif //SIMULATION_H