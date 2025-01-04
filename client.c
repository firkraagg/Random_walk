#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "Menu.h"
#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

Simulation* receive_simulation(SOCKET server_socket) {
    Simulation* sim = (Simulation*)malloc(sizeof(Simulation));
    sim->world_ = (World*)malloc(sizeof(World));
    memset(sim->world_, 0, sizeof(World));

    recv(server_socket, (char*)&sim->singlePlayer_, sizeof(sim->singlePlayer_), 0);
    recv(server_socket, (char*)&sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);
    recv(server_socket, (char*)&sim->world_->width_, sizeof(sim->world_->width_), 0);
    recv(server_socket, (char*)&sim->world_->height_, sizeof(sim->world_->height_), 0);
    recv(server_socket, (char*)&sim->mode_, sizeof(sim->mode_), 0);
    recv(server_socket, (char*)&sim->numReplications_, sizeof(sim->numReplications_), 0);
    recv(server_socket, (char*)&sim->K_, sizeof(sim->K_), 0);
    recv(server_socket, (char*)&sim->probabilities_, sizeof(sim->probabilities_), 0);

    char inputFileName[BUFFER_SIZE];
    memset(inputFileName, 0, BUFFER_SIZE);
    recv(server_socket, inputFileName, BUFFER_SIZE, 0);
    sim->world_->inputFileName_ = strdup(inputFileName);

    char outputFileName[BUFFER_SIZE];
    memset(outputFileName, 0, BUFFER_SIZE);
    recv(server_socket, outputFileName, BUFFER_SIZE, 0);
    sim->world_->outputFileName_ = strdup(outputFileName);

    recv(server_socket, (char*)&sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);

    return sim;
}

void send_command(SOCKET server_socket, const char* command, void* argument, char* response) {
    send(server_socket, command, strlen(command), 0);
    send(server_socket, argument, sizeof(SimulationInputs), 0);
    memset(response, 0, BUFFER_SIZE);
    recv(server_socket, response, BUFFER_SIZE, 0);
}

int main() {
    srand(time(NULL));
    
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int option;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Nepodarilo sa inicializovat Winsock. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Nepodarilo sa vytvorit socket. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Pripojenie zlyhalo. Kod chyby: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Pripojeny na server.\n");

    while (1) {
        display_menu();
        int choice;
        scanf("%d", &choice);
        printf("\n");
        getchar();

        switch (choice) {
            case 1:
                SimulationInputs* sp = input_from_user();
                send_command(client_socket, "CREATE_SIMULATION", sp, buffer);
                Simulation* sim = receive_simulation(client_socket);
                if (sim) {
                    run_simulation(sim);
                }
                break;
            case 2:
                send_command(client_socket, "CONNECT_SIMULATION", NULL, buffer);
                Simulation* simulation = receive_simulation(client_socket);
                if (simulation) {
                    printf("Pripojený k existujúcej simulácii!\n");
                    run_simulation(simulation);
                } else {
                    printf("Nie je žiadna aktívna simulácia na pripojenie alebo už skončila.\n");
                }
                break;
            case 3:
                run_simulation(recreate_simulation());
                break;
            case 4:
                send_command(client_socket, "EXIT", NULL, buffer);
                printf("Ukoncujem...\n");
                closesocket(client_socket);
                WSACleanup();
                return 0;
            default:
                printf("Zla volba. Skus znova.\n");
                break;
        }
    }

    return 0;
}
