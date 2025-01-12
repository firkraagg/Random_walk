#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

#include "client.h"

typedef struct {
    pthread_mutex_t world_mutex;
    pthread_cond_t world_cond;
    int world_ready;
    int end_signal_received;
    int is_connected;
} ClientState;

typedef struct {
    int client_socket;
    int count;
    World* world;
    ClientState* state;
} ThreadArgs;

void run_server() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Nepodarilo sa vytvoriť proces servera");
        exit(1);
    }

    if (pid == 0) {
        start_server();
        perror("Nepodarilo sa spustiť server");
        exit(1);
    }

    sleep(1);
}

int connect_to_server() {
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Pripojenie zlyhalo");
        close(client_socket);
        return -1;
    }

    printf(">>> [KLIENT] - Pripojený na server.\n");
    return client_socket;
}

void send_command(int server_socket, const char* command, SimulationInputs* sp) {
    Message message;
    memset(&message, 0, sizeof(Message));
    strncpy(message.command, command, BUFFER_SIZE - 1);
    if (sp) {
        message.sp = *sp;
    }

    int bytes_sent = send(server_socket, &message, sizeof(Message), 0);
}

Simulation* receive_simulation(int server_socket) {
    Simulation* sim = (Simulation*)malloc(sizeof(Simulation));
    sim->world_ = (World*)malloc(sizeof(World));
    memset(sim->world_, 0, sizeof(World));

    int bytes_received = recv(server_socket, &sim->singlePlayer_, sizeof(sim->singlePlayer_), 0);
    bytes_received = recv(server_socket, &sim->world_->worldType_, sizeof(sim->world_->worldType_), 0);
    bytes_received = recv(server_socket, &sim->world_->width_, sizeof(sim->world_->width_), 0);
    bytes_received = recv(server_socket, &sim->world_->height_, sizeof(sim->world_->height_), 0);
    bytes_received = recv(server_socket, &sim->mode_, sizeof(sim->mode_), 0);
    bytes_received = recv(server_socket, &sim->numReplications_, sizeof(sim->numReplications_), 0);
    bytes_received = recv(server_socket, &sim->K_, sizeof(sim->K_), 0);
    bytes_received = recv(server_socket, &sim->probabilities_, sizeof(sim->probabilities_), 0);

    char inputFileName[BUFFER_SIZE];
    memset(inputFileName, 0, BUFFER_SIZE);
    bytes_received = recv(server_socket, inputFileName, BUFFER_SIZE, 0);
    sim->world_->inputFileName_ = strdup(inputFileName);

    char outputFileName[BUFFER_SIZE];
    memset(outputFileName, 0, BUFFER_SIZE);
    bytes_received = recv(server_socket, outputFileName, BUFFER_SIZE, 0);
    sim->world_->outputFileName_ = strdup(outputFileName);

    sim->world_->grid_ = (char**)malloc(sim->world_->height_ * sizeof(char*));
    for (int i = 0; i < sim->world_->height_; i++) {
        sim->world_->grid_[i] = (char*)malloc(sim->world_->width_ * sizeof(char));
    }

    return sim;
}

int receive_world(int client_socket, World* world) {
    fd_set read_fds, write_fds;
    struct timeval timeout;

    if (world->grid_) {
        for (int i = 0; i < world->height_; i++) {
            free(world->grid_[i]);
        }
        free(world->grid_);
    }

    world->grid_ = malloc(world->height_ * sizeof(char*));
    if (!world->grid_) {
        perror("[CLIENT] Alokacia pamate pre hracie pole zlyhala");
        return -1;
    }

    for (int i = 0; i < world->height_; i++) {
        world->grid_[i] = malloc(world->width_ * sizeof(char));
        if (!world->grid_[i]) {
            perror("[CLIENT] Alokacia pamate pre riadok zlyhala");
            return -1;
        }

        memset(world->grid_[i], 0, world->width_ * sizeof(char));

        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int select_result = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result == -1) {
            perror("[CLIENT] Problem so selectom");
            return -1;
        } else if (select_result == 0) {
            fprintf(stderr, "[CLIENT] Cakanie na socket aby bol citatelny zlyhalo\n");
            return -1;
        }

        int bytes_to_receive = world->width_ * sizeof(char);
        int received_bytes = 0;
        do {
            int result = recv(client_socket, world->grid_[i] + received_bytes, bytes_to_receive - received_bytes, 0);
            if (result == -1) {
                perror("[CLIENT] Problem s prijatim riadku");
                close(client_socket);
                return -1;
            }
            received_bytes += result;
        } while (received_bytes != bytes_to_receive);

        FD_ZERO(&write_fds);
        FD_SET(client_socket, &write_fds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        select_result = select(client_socket + 1, NULL, &write_fds, NULL, &timeout);
        if (select_result == -1) {
            perror("[CLIENT] Problem so selectom");
            return -1;
        } else if (select_result == 0) {
            fprintf(stderr, "[CLIENT] Cakanie na socket aby bol zapisovatelny zlyhalo\n");
            return -1;
        }
    }
    
    return 0;
}

void* receive_world_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int client_socket = args->client_socket;
    World* world = args->world;
    int count = args->count;
    ClientState* state = args->state;

    while (1) {
        pthread_mutex_lock(&state->world_mutex);
        if (state->end_signal_received) {
            pthread_mutex_unlock(&state->world_mutex);
            break;
        }
        pthread_mutex_unlock(&state->world_mutex);

        if (receive_world(client_socket, world) == -1) {
            printf("Nepodarilo sa prijat riadok.\n");
            break;
        }

        state->world_ready = 1;
        pthread_cond_signal(&state->world_cond);

        char end_signal;
        int bytes_to_receive = sizeof(end_signal);
        int received_bytes = 0;
        usleep(100000);
        count--;
        if (count <= 1) {
            state->end_signal_received = 1;
            break;
        }
    }

    state->end_signal_received = 1;
    return NULL;
}

void* print_world_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    World* world = args->world;
    ClientState* state = args->state;

    while (1) {
        pthread_mutex_lock(&state->world_mutex);
        while (!state->world_ready && !state->end_signal_received) {
            pthread_cond_wait(&state->world_cond, &state->world_mutex);
        }

        if (state->end_signal_received) {
            pthread_mutex_unlock(&state->world_mutex);
            break;
        }

        print_world(world);
        state->world_ready = 0;
        pthread_mutex_unlock(&state->world_mutex);

        usleep(100000);
    }
    return NULL;
}

void start_client() {
    ClientState state = {
        .world_mutex = PTHREAD_MUTEX_INITIALIZER,
        .world_cond = PTHREAD_COND_INITIALIZER,
        .world_ready = 0,
        .end_signal_received = 0,
        .is_connected = 0
    };

    int client_socket = -1;
    char buffer[BUFFER_SIZE];

    while (1) {
        display_menu();
        int choice;
        scanf("%d", &choice);
        printf("\n");
        getchar();

        switch (choice) {
            case 1: {
                SimulationInputs* sp = input_from_user();
                run_server();
                client_socket = connect_to_server();
                if (client_socket < 0) {
                    printf(">>> [KLIENT] - Server neexistuje alebo sa nepodarilo pripojiť. <<<\n");
                    free(sp);
                    break;
                }
                state.is_connected = 1;
                send_command(client_socket, "CREATE_SIMULATION", sp);
                
                Simulation* sim = receive_simulation(client_socket);
                if (sim) {
                    int count = sim->K_ * sim->numReplications_;
                    ThreadArgs args = {client_socket, count, sim->world_, &state};
                    pthread_t receive_thread, print_thread;
                    pthread_create(&receive_thread, NULL, receive_world_thread, &args);
                    pthread_create(&print_thread, NULL, print_world_thread, &args);

                    while (state.end_signal_received == 0){
                        usleep(100000);
                    }
                    
                    pthread_join(receive_thread, NULL);
                    pthread_join(print_thread, NULL);

                    free_simulation(sim);
                    
                } else {
                    printf(">>>[KLIENT] - Nepodarilo sa vytvoriť simuláciu. <<<\n");
                }

                free(sp);
                close(client_socket);
                exit(0);
            }
            case 2: {
                client_socket = connect_to_server();
                send_command(client_socket, "CONNECT_SIMULATION", NULL);
                Simulation* simulation = receive_simulation(client_socket);
                if (simulation) {
                    printf("Pripojený k existujúcej simulácii!\n");
                } else {
                    printf("Nie je žiadna aktívna simulácia na pripojenie alebo už skončila.\n");
                }
                close(client_socket);
                break;
            }
            case 3:
                //run_simulation(recreate_simulation());
                break;
            case 4:
                printf(">>> [KLIENT] - Ukončujem... <<<\n");
                close(client_socket);
                return;
            default:
                printf(">>> [KLIENT] - Zlá voľba. Skús znova. <<<\n");
                break;
        }
    }
}