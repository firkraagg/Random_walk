#include "Simulation.h"

void create_simulation(SimulationInputs* sp, Simulation* sim) {
    sim->world_ = (World*)malloc(sizeof(World));
    sim->world_->width_ = sp->worldWidth;
    sim->world_->height_ = sp->worldHeight;
    sim->singlePlayer_ =sp->singlePlayer;
    switch (sp->worldType)
    {
    case 0:
        sim->world_->worldType_ = WORLD_EMPTY;
        break;
    case 1:
        sim->world_->worldType_ = WORLD_OBSTACLES_GENERATED;
        break;

    case 2:
        sim->world_->worldType_ = WORLD_OBSTACLES_FILE;
        break;
    
    default:
        break;
    }

    sim->mode_ = sp->mode;
    sim->numReplications_ = sp->numReplications;
    sim->K_ = sp->K;
    memcpy(sim->probabilities_, sp->probabilities, sizeof(sim->probabilities_));
    sim->world_->inputFileName_ = sp->inputFileName;
    sim->world_->outputFileName_ = sp->outputFileName;
}

SimulationInputs* input_from_user() {
    SimulationInputs* sp = (SimulationInputs*)malloc(sizeof(SimulationInputs));
    sp->singlePlayer = choose_player_mode();
    sp->worldType = choose_world_type();
    sp->worldWidth = 0;
    sp->worldHeight = 0;
    memset(sp->inputFileName, 0, sizeof(sp->inputFileName));
    memset(sp->outputFileName, 0, sizeof(sp->outputFileName));
    if (sp->worldType == 2)
    {
        char* inputFileName = choose_input_file();
        strncpy(sp->inputFileName, inputFileName, sizeof(sp->inputFileName) - 1);
        free(inputFileName);    
    } else {
        get_size(&sp->worldWidth, &sp->worldHeight);
    }
    
    sp->mode = choose_mode();
    sp->numReplications = choose_number_of_replications();
    sp->K = number_of_steps();
    float* probabilities = choose_probabilities();
    memcpy(sp->probabilities, probabilities, sizeof(sp->probabilities));
    free(probabilities);
    char* outputFileName = choose_output_file();
    strncpy(sp->outputFileName, outputFileName, sizeof(sp->outputFileName) - 1);
    free(outputFileName);
    return sp;
}

void run_simulation(Simulation* simulation, int client_socket) {
    printf("========== SIMULÁCIA ==========\n");
    initialize_simulation(simulation);
    if (simulation->mode_ == INTERACTIVE_MODE) {
        for (size_t i = 0; i < simulation->numReplications_; i++) {
            perform_replication(simulation, client_socket, i);
        }
    } else if (simulation->mode_ == SUMMARY_MODE_WITHOUT_K || simulation->mode_ == SUMMARY_MODE_WITH_K) {
        print_world_summary(simulation->world_, simulation->mode_);
    }

    finalize_simulation(simulation, client_socket);
}

void initialize_simulation(Simulation* simulation) {
    initialize_world(simulation->world_, simulation->mode_, simulation->world_->worldType_, simulation->world_->width_, simulation->world_->height_, simulation->K_, simulation->probabilities_);
}

void perform_replication(Simulation* simulation, int client_socket, size_t replication_index) {
    starting_position(simulation->world_, &simulation->world_->pedestrian_->x_, &simulation->world_->pedestrian_->y_);
    reset_world(simulation->world_);
    for (size_t j = 0; j < simulation->K_; j++) {
        if (j == 0) {
            printf(">>> Replikacia: %d / %d <<<\n", (int)replication_index + 1, simulation->numReplications_);
            printf("\n\tSvet:\n");
        }
        sleep(1);
        send_world(client_socket, simulation->world_);
        move_pedestrian(simulation->world_, simulation->probabilities_);
        fflush(stdin);

        char ack;
        if (recv(client_socket, &ack, sizeof(ack), 0) == -1) {
            perror("Chyba pri prijimani potvrdenia");
            return;
        }
        if (ack != 'A') {
            fprintf(stderr, "Necakane potvrdenie: %c\n", ack);
            return;
        }
    }
    if (replication_index + 1 != simulation->numReplications_) {
        printf("-------------------------------\n");
    }
}

void finalize_simulation(Simulation* simulation, int client_socket) {
    save_simulation_results(simulation, simulation->world_->outputFileName_);

    char end_signal = 'E';
    send(client_socket, &end_signal, sizeof(end_signal), 0);

    printf("========== KONIEC SIMULÁCIE ==========\n\n");
}

void send_world(int client_socket, World* world) {
    char start_signal = 'S';
    if (send(client_socket, &start_signal, sizeof(start_signal), 0) == -1) {
        perror("Problem zo zaslanim startovacieho signalu");
        return;
    }

    int num_rows = world->height_;
    if (send(client_socket, &num_rows, sizeof(num_rows), 0) == -1) {
        perror("Problem s odoslanim poctu riadkov");
        return;
    }

    for (int i = 0; i < num_rows; i++) {
        if (send(client_socket, world->grid_[i], world->width_ * sizeof(char), 0) == -1) {
            perror("Problem so zaslanim riadka");
            return;
        }

        char ack;
        if (recv(client_socket, &ack, sizeof(ack), 0) == -1) {
            perror("Problem s prijatim potvrdenia");
            return;
        }
    }
}

Simulation* recreate_simulation() {
    printf("======== OBNOVENIE SIMULACIE =========\n");

    Simulation* simulation = (Simulation*)malloc(sizeof(Simulation));
    simulation->world_ = (World*)malloc(sizeof(World));

    printf(">>> Zadajte nazov suboru, z ktoreho ma byt simulacia nacitana <<<\n");
    printf("- Nazov: ");
    char inputFileName[100];
    scanf("%s", inputFileName);
    printf("\n");

    printf(">>> Zadajte pocet replikacii<<<\n");
    printf("- Pocet: ");
    int numReplications;
    scanf("%d", &numReplications);
    printf("\n");

    printf(">>> Zadajte nazov suboru, do ktoreho sa maju vysledky ulozit <<<\n");
    printf("- Nazov: ");
    char outputFileName[100];
    scanf("%s", outputFileName);
    printf("\n");

    simulation->numReplications_ = numReplications;
    simulation->world_->outputFileName_ = strdup(outputFileName);
    load_simulation_results(simulation, inputFileName);

    return simulation;
}

void load_simulation_results(Simulation* simulation, const char* fileName) {
    FILE* file = fopen(fileName, "r");

    if (file == NULL) {
        printf("Nebolo mozne otvorit subor s nazvom: %s\n", fileName);
        return;
    }

    for (int i = 0; i < 4; i++) {
        fscanf(file, "%f", &simulation->probabilities_[i]);
    }

    int width, height;
    fscanf(file, "%d %d", &width, &height);
    int K;
    fscanf(file, "%d", &K);
    char worldType[50];
    fscanf(file, "%s", worldType);

    simulation->world_->width_ = width;
    simulation->world_->height_ = height;
    simulation->K_ = K;
    simulation->singlePlayer_ = true;
    simulation->mode_ = INTERACTIVE_MODE;
    simulation->world_->grid_ = (char**)malloc(height * sizeof(char*));
    simulation->world_->pedestrian_ = (Pedestrian*)malloc(sizeof(Pedestrian));

    if (strcmp(worldType, "WORLD_EMPTY") == 0) {
        simulation->world_->worldType_ = WORLD_EMPTY;
    } else if (strcmp(worldType, "WORLD_OBSTACLES_FILE") == 0) {
        simulation->world_->worldType_ = WORLD_OBSTACLES_FILE;
    } else {
        simulation->world_->worldType_ = WORLD_OBSTACLES_GENERATED;
    }

    calculate_center(simulation->world_);

    for (int i = 0; i < height; i++) {
        simulation->world_->grid_[i] = (char*)malloc(width * sizeof(char));
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fscanf(file, " %c", &simulation->world_->grid_[i][j]);

            if (simulation->world_->grid_[i][j] == 'C') {
                simulation->world_->pedestrian_->x_ = j;
                simulation->world_->pedestrian_->y_ = i;
                simulation->world_->pedestrian_->startX_ = j;
                simulation->world_->pedestrian_->startY_ = i;
            }
        }
    }

    fclose(file);
}

void save_simulation_results(Simulation* simulation, const char* fileName) {
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Nepodarilo sa otvorit subor na zapis: %s\n", fileName);
        return;
    }

    for (int i = 0; i < 4; i++) {
        fprintf(file, "%.4f", simulation->probabilities_[i]);
        if (i < 3) {
            fprintf(file, " ");
        }
    }
    fprintf(file, "\n");

    fprintf(file, "%d %d\n", simulation->world_->width_, simulation->world_->height_);
    fprintf(file, "%d\n", simulation->K_);
    fprintf(file, "%s\n", world_type_to_string(simulation->world_->worldType_));
    
    for (int i = 0; i < simulation->world_->height_; i++) {
        for (int j = 0; j < simulation->world_->width_; j++) {
            fprintf(file, "%c", simulation->world_->grid_[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}


float* choose_probabilities() {
    printf(">>> Nastavte pravdepodobnosti pre pohyb chodca <<<\n");
    printf("(Vsetky hodnoty spolu musia byt rovne 1.0:)\n");
    
    float* array = (float*)malloc(4 * sizeof(float));
    float sum;
    do {
        printf("- Pohyb dolava: ");
        scanf("%f", &array[0]);
    
        printf("- Pohyb doprava: ");
        scanf("%f", &array[1]);

        printf("- Pohyb hore: ");
        scanf("%f", &array[2]);

        printf("- Pohyb dole: ");
        scanf("%f", &array[3]);

        sum = array[0] + array[1] + array[2] + array[3];

        if (fabs(sum - 1.0f) > 1e-6) {
            printf("Chyba: Sucet pravdepodobnosti musi byt 1.0. Skuste to znova.\n");
        }
    } while (fabs(sum - 1.0f) > 1e-6);
    
    printf("\n");
    return array;
}

int choose_number_of_replications() {
    printf(">>> Nastavte pocet replikacii <<<\n");
    printf("- Pocet replikacii: ");
    int numReplications;
    scanf("%d", &numReplications);
    printf("\n");

    return numReplications;
}

int choose_world_type() {
    printf("========== TYP SVETA ==========\n");
    printf("1. Svet bez prekazok\n");
    printf("2. Svet s prekazkami\n");
    printf("===============================\n");
    printf("Vyberte typ sveta: ");
    int result;
    scanf("%d", &result);
    printf("\n");
    switch (result)
    {
    case 1:
        return 0;
    
    default:
        return choose_world_with_obstacles();
        break;
    }
    return result;
}

int choose_world_with_obstacles() {
    printf("======= MOZNOSTI VYTVORENIA SVETA =======\n");
    printf("1. Nahodna generacia\n");
    printf("2. Nacitane zo suboru\n");
    printf("=========================================\n");
    printf("Vyberte moznost: ");

    int result;
    scanf("%d", &result);
    printf("\n");
    return result;
}

void get_size(int* x, int* y) {
    printf(">>> Zadajte rozmery sveta <<<\n");
    printf("- Pocet riadkov: ");
    scanf("%d", x);
    printf("- Pocet stlpcov: ");
    scanf("%d", y);
    printf("\n");
}

char* choose_input_file() {
    printf(">>> Zadajte nazov vstupneho suboru <<<\n");
    printf("- Nazov: ");
    char* fileName = malloc(100 * sizeof(char));
    scanf("%99s", fileName);
    printf("\n");

    return fileName;
}

char* choose_output_file() {
    printf(">>> Zadajte nazov vystupneho suboru <<<\n");
    printf("- Nazov: ");
    char* fileName = (char*)malloc(100 * sizeof(char)); 
    scanf("%s", fileName);
    printf("\n");

    return fileName;
}

bool choose_player_mode() {
    printf("========= REZIM SIMULACIE ==========\n");
    printf("1. Rezim pre jedneho hraca\n");
    printf("2. Rezim pre viac hracov (nefunkcny)\n");
    printf("====================================\n");
    printf("Vyberte rezim: ");
    int result;
    scanf("%d", &result);
    printf("\n");
    switch (result)
    {
    case 1:
        return true;
    
    case 2:
        return false;

    default:
        return true;
    }
}

int number_of_steps() {
    printf(">>> Zadajte maximalny pocet krokov <<<\n");
    printf("- Pocet krokov: ");
    int numberOfSteps;
    scanf("%d", &numberOfSteps);
    printf("\n");

    return numberOfSteps;
}

SimulationMode choose_mode() {
    printf("=========== MOD SIMULACIE ============\n");
    printf("1. Interaktivny\n");
    printf("2. Sumarny (priemerny pocet posunov)\n");
    printf("3. Sumarny (pravdepodobnost dosiahnutia stredu s najviac K posunmi)\n");
    printf("======================================\n");
    printf("Zvolte mod: ");
    int choice;
    scanf("%d", &choice);
    printf("\n");
    switch (choice)
    {
    case 1:
        return INTERACTIVE_MODE;
    
    case 2:
        return SUMMARY_MODE_WITHOUT_K;

    case 3:
        return SUMMARY_MODE_WITH_K;

    default:
        return INTERACTIVE_MODE;
    }
}

void free_simulation(Simulation* simulation) {
    if (simulation == NULL) return;
    if (simulation->world_) {
        if (simulation->world_->grid_) {
            for (int i = 0; i < simulation->world_->height_; i++) {
                free(simulation->world_->grid_[i]);
            }
            free(simulation->world_->grid_);
        }
        if (simulation->world_->initial_grid_) {
            for (int i = 0; i < simulation->world_->height_; i++) {
                free(simulation->world_->initial_grid_[i]);
            }
            free(simulation->world_->initial_grid_);
        }
        if (simulation->world_->stepsGrid_) {
            for (int i = 0; i < simulation->world_->height_; i++) {
                free(simulation->world_->stepsGrid_[i]);
            }
            free(simulation->world_->stepsGrid_);
        }
        if (simulation->world_->probabilityGrid_) {
            for (int i = 0; i < simulation->world_->height_; i++) {
                free(simulation->world_->probabilityGrid_[i]);
            }
            free(simulation->world_->probabilityGrid_);
        }
        if (simulation->world_->inputFileName_) {
            free(simulation->world_->inputFileName_);
        }
        if (simulation->world_->outputFileName_) {
            free(simulation->world_->outputFileName_);
        }
        if (simulation->world_->pedestrian_) {
            free(simulation->world_->pedestrian_);
        }
        if (simulation->world_->initial_pedestrian_) {
            free(simulation->world_->initial_pedestrian_);
        }
        free(simulation->world_);
    }
    free(simulation);
}