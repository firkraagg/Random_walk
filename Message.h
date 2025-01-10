#ifndef MESSAGE_H
#define MESSAGE_H

#include "Simulation.h"

typedef struct {
    char command[1024];
    SimulationInputs sp;
} Message;

#endif // MESSAGE_H