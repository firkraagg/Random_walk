#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Menu.h"
#include "Message.h"

void start_client();
void* print_world_thread(void* arg);
void* receive_world_thread(void* arg);
int receive_world(int client_socket, World* world);
Simulation* receive_simulation(int server_socket);
void send_command(int server_socket, const char* command, SimulationInputs* sp);
int connect_to_server();
void start_server();

#endif //CLIENT_H