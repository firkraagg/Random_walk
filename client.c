#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

#include "client.h"

pthread_mutex_t world_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t world_cond = PTHREAD_COND_INITIALIZER;
int world_ready = 0;
int end_signal_received = 0;
int is_connected = 0;

typedef struct {
    int client_socket;
    World* world;
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
    printf(">>> [KLIENT] - Odoslaný príkaz '%s' s argumentmi: %d bajtov\n", command, bytes_sent);
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
    char start_signal;
    if (recv(client_socket, &start_signal, sizeof(start_signal), 0) == -1 || start_signal != 'S') {
        end_signal_received = 1;
        return -1;
    }

    int num_rows;
    if (recv(client_socket, &num_rows, sizeof(num_rows), 0) == -1) {
        perror("Chyba pri prijimani poctu riadkov");
        return -1;
    }

    if (world->grid_) {
        for (int i = 0; i < world->height_; i++) {
            free(world->grid_[i]);
        }
        free(world->grid_);
    }

    world->grid_ = malloc(num_rows * sizeof(char*));
    for (int i = 0; i < num_rows; i++) {
        world->grid_[i] = malloc(world->width_ * sizeof(char));
    }

    for (int i = 0; i < num_rows; i++) {
        if (recv(client_socket, world->grid_[i], world->width_ * sizeof(char), 0) == -1) {
            perror("Chyba pri prijimani riadku.");
            return -1;
        }

        char ack = 'A';
        if (send(client_socket, &ack, sizeof(ack), 0) == -1) {
            perror("Chyba pri odosielani potvrdenia.");
            return -1;
        }
    }
    char ack = 'A';
    if (send(client_socket, &ack, sizeof(ack), 0) == -1) {
        perror("Chyba pri odosielani potvrdenia.");
        return 0;
    }
}

void* receive_world_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int client_socket = args->client_socket;
    World* world = args->world;

    while (1) {
        pthread_mutex_lock(&world_mutex);
        if (end_signal_received) {
            pthread_mutex_unlock(&world_mutex);
            break;
        }
        pthread_mutex_unlock(&world_mutex);
        
        if (receive_world(client_socket, world) == -1) {
            break;
        }
        
        world_ready = 1;
        pthread_cond_signal(&world_cond);

        usleep(100000);
    }
    return NULL;
}

void* print_world_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    World* world = args->world;

    while (1) {
        pthread_mutex_lock(&world_mutex);
        while (!world_ready && !end_signal_received) {
            pthread_cond_wait(&world_cond, &world_mutex);
        }

        if (end_signal_received) {
            pthread_mutex_unlock(&world_mutex);
            break;
        }

        print_world(world);
        world_ready = 0;
        pthread_mutex_unlock(&world_mutex);

        usleep(100000);
    }
    return NULL;
}

void start_client() {
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
                is_connected = 1;
                send_command(client_socket, "CREATE_SIMULATION", sp);
                
                Simulation* sim = receive_simulation(client_socket);
                if (sim) {
                    ThreadArgs args = {client_socket, sim->world_};
                    pthread_t receive_thread, print_thread;
                    pthread_create(&receive_thread, NULL, receive_world_thread, &args);
                    pthread_create(&print_thread, NULL, print_world_thread, &args);

                    while (end_signal_received == 0){}
                    
                    pthread_join(receive_thread, NULL);
                    pthread_join(print_thread, NULL);

                    free_simulation(sim);
                } else {
                    printf(">>>[KLIENT] - Nepodarilo sa vytvoriť simuláciu. <<<\n");
                }

                close(client_socket);
                free(sp);
                break;
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
                if (!is_connected) {
                    client_socket = connect_to_server();
                    if (client_socket < 0) {
                        printf(">>> [KLIENT] - Server neexistuje alebo sa nepodarilo pripojiť na server. <<<\n");
                        break;
                    }
                    is_connected = 1;
                }
                
                send_command(client_socket, "SHUTDOWN_SERVER", NULL);
                printf(">>> [KLIENT] - Ukončujem... <<<\n");
                close(client_socket);
                is_connected = 0;
                return;
            case 5:
                printf(">>> [KLIENT] - Ukončujem... <<<\n");
                close(client_socket);
                return;
            default:
                printf(">>> [KLIENT] - Zlá voľba. Skús znova. <<<\n");
                break;
        }
    }
}