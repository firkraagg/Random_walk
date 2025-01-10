#define PORT 8080
#define BUFFER_SIZE 1024

#include "server.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Simulation* active_simulation = NULL;

void send_simulation(int client_socket, Simulation* sim) {
    int bytes_sent = send(client_socket, &sim->singlePlayer_, sizeof(sim->singlePlayer_), 0);
    bytes_sent = send(client_socket, &sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);
    bytes_sent = send(client_socket, &sim->world_->width_, sizeof(sim->world_->width_), 0);
    bytes_sent = send(client_socket, &sim->world_->height_, sizeof(sim->world_->height_), 0);
    bytes_sent = send(client_socket, &sim->mode_, sizeof(sim->mode_), 0);
    bytes_sent = send(client_socket, &sim->numReplications_, sizeof(sim->numReplications_), 0);
    bytes_sent = send(client_socket, &sim->K_, sizeof(sim->K_), 0);
    bytes_sent = send(client_socket, &sim->probabilities_, sizeof(sim->probabilities_), 0);
    bytes_sent = send(client_socket, sim->world_->inputFileName_, strlen(sim->world_->inputFileName_) + 1, 0);
    bytes_sent = send(client_socket, sim->world_->outputFileName_, strlen(sim->world_->outputFileName_) + 1, 0);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    Message message;
    memset(&message, 0, sizeof(Message));

    int bytes_received = recv(client_socket, &message, sizeof(Message), 0);
    if (bytes_received <= 0) {
        printf(">>> [SERVER] - Klient bol odpojený.\n");
        close(client_socket);
        pthread_exit(NULL);
    }

    if (strcmp(message.command, "CREATE_SIMULATION") == 0) {
        SimulationInputs sp = message.sp;

        pthread_mutex_lock(&mutex);
        if (active_simulation) {
            free_simulation(active_simulation);
        }

        Simulation* simulation = (Simulation*)malloc(sizeof(Simulation));
        memset(simulation, 0, sizeof(Simulation));
        create_simulation(&sp, simulation);
        active_simulation = simulation;
        pthread_mutex_unlock(&mutex);

        send_simulation(client_socket, simulation);
        run_simulation(simulation, client_socket);
    } else if (strcmp(message.command, "SHUTDOWN_SERVER") == 0)
    {
        printf(">>> [SERVER] - Server sa vypina... <<<\n");
        close(client_socket);
        pthread_exit(NULL);
        exit(0);
    }
    

    close(client_socket);
    pthread_exit(NULL);
}

void start_server() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        return;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Vazba zlyhala");
        close(server_socket);
        return;
    }

    if (listen(server_socket, 3) < 0) {
        perror("Pocuvanie servera zlyhalo");
        close(server_socket);
        return;
    }

    printf(">>> [SERVER] - Server čaká na pripojenie klienta na porte %d...\n", PORT);

    while (1) {
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (*client_socket < 0) {
            perror("Prijatie klienta zlyhalo");
            free(client_socket);
            continue;
        }

        printf(">>> [SERVER] - Klient pripojený.\n");

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("Nepodarilo sa vytvoriť vlákno");
            free(client_socket);
        }
        pthread_detach(thread_id);
    }

    close(server_socket);
}