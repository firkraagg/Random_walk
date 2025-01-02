#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include "Pedestrian.h"

typedef struct World World;

typedef enum SimulationMode {
    INTERACTIVE_MODE,
    SUMMARY_MODE_WITH_K,
    SUMMARY_MODE_WITHOUT_K
}   SimulationMode;

typedef struct Simulation {
    int numReplications_;
    float probabilities_[4];
    int K_;
    SimulationMode mode_;
    bool singlePlayer_;
    World* world_;
} Simulation;

Simulation* create_simulation();
void run_simulation(Simulation* simulation);
void save_simulation_results(Simulation* simulation, const char* fileName);
void load_simulation_results(Simulation* simulation, const char* fileName);
Simulation* recreate_simulation();
void free_simulation(Simulation* simulation);

float* choose_probabilities();
int choose_number_of_replications();
void choose_world_type(World* world);
char* choose_input_file();
char* choose_output_file();
void choose_world_with_obstacles(World* world);
bool choose_player_mode();
int number_of_steps();
SimulationMode choose_mode();
void get_size(World* world);

#endif //SIMULATION_H