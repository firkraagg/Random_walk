#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include "World.h"
#include "Simulation.h"
#include <time.h>

struct World;  
typedef struct World World;

typedef struct Pedestrian {
    int startX_;
    int startY_;
    int x_;
    int y_;
} Pedestrian;

Pedestrian* create_pedestrian();
void free_pedestrian(Pedestrian* pedestrian);
void initialize_position(World* world);
int random_start_x(World* world);
int random_start_y(World* world);
void move_pedestrian(World* world, float probabilities[4]);

#endif //PEDESTRIAN_H