#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include "Simulation.h"
#include "Message.h"

void start_server();
void* handle_client(void* arg);
void send_simulation(int client_socket, Simulation* sim);

#endif //SERVER_H