#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "Simulation.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

Simulation* active_simulation = NULL;

void send_simulation(SOCKET client_socket, Simulation* sim) {
    send(client_socket, (char*)&sim->singlePlayer_, sizeof(sim->singlePlayer_), 0);
    send(client_socket, (char*)&sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);
    send(client_socket, (char*)&sim->world_->width_, sizeof(sim->world_->width_), 0);
    send(client_socket, (char*)&sim->world_->height_, sizeof(sim->world_->height_), 0);
    send(client_socket, (char*)&sim->mode_, sizeof(sim->mode_), 0);
    send(client_socket, (char*)&sim->numReplications_, sizeof(sim->numReplications_), 0);
    send(client_socket, (char*)&sim->K_, sizeof(sim->K_), 0);
    send(client_socket, (char*)&sim->probabilities_, sizeof(sim->probabilities_), 0);
    send(client_socket, sim->world_->inputFileName_, strlen(sim->world_->inputFileName_) + 1, 0);
    send(client_socket, sim->world_->outputFileName_, strlen(sim->world_->outputFileName_) + 1, 0);

    send(client_socket, (char*)&sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);
}

int handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Klient bol odpojeny.\n");
        return 1;
    }
    printf("Prijaty prikaz: %s\n", buffer);

    if (strcmp(buffer, "CREATE_SIMULATION") == 0) {
        SimulationInputs sp;
        memset(&sp, 0, sizeof(SimulationInputs));
        recv(client_socket, (char*)&sp, sizeof(SimulationInputs), 0);

        if (active_simulation) {
            free_simulation(active_simulation);
        }

        Simulation* simulation = (Simulation*)malloc(sizeof(Simulation));
        memset(simulation, 0, sizeof(Simulation));

        create_simulation(&sp, simulation);
        active_simulation = simulation;
        send_simulation(client_socket, simulation);

        const char* response = "Simulacia bola uspesne vytvorena.\n";
        send(client_socket, response, strlen(response), 0);

    } else if (strcmp(buffer, "CONNECT_SIMULATION") == 0) {
        if (active_simulation) {
            printf("Pripojenie k existujucej simulacii.\n");
            send_simulation(client_socket, active_simulation);
        } else {
            const char* response = "Ziadna aktivna simulacia na pripojenie.\n";
            send(client_socket, response, strlen(response), 0);
        }
    } else if (strcmp(buffer, "EXIT") == 0) {
        printf("Ukoncujem server...\n");
        return 0;
    }
    return 1;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Nepodarilo sa inicializovat Winsock. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Nepodarilo sa vytvorit socket. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Nepodarilo sa priradit socket k adrese. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    listen(server_socket, 3);
    printf("Server caka na pripojenie klienta na porte %d...\n", PORT);

    int running = 1;
    while (running) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            printf("Chyba pri pripojeni klienta. Error Code: %d\n", WSAGetLastError());
            continue;
        }
        printf("Klient pripojeny.\n");

        running = handle_client(client_socket);
        closesocket(client_socket);
    }

    free_simulation(active_simulation);
    closesocket(server_socket);
    WSACleanup();
    printf("Server bol ukonceny.\n");

    return 0;
}