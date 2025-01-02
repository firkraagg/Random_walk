#include "Simulation.h"

Simulation* create_simulation() {
    Simulation* sim = (Simulation*)malloc(sizeof(Simulation));
    sim->world_ = (World*)malloc(sizeof(World));
    sim->world_->pedestrian_ = create_pedestrian();
    printf("-----------Vytvaranie simulacie-----------\n");
    sim->singlePlayer_ = choose_player_mode();
    choose_world_type(sim->world_);
    sim->mode_ = choose_mode(sim);
    sim->numReplications_ = choose_number_of_replications();
    sim->K_ = number_of_steps();
    memcpy(sim->probabilities_, choose_probabilities(), sizeof(sim->probabilities_));
    sim->outputFileName_ = choose_output_file();
    return sim;
}

void run_simulation(Simulation* simulation) {
    initialize_world(simulation->world_, simulation->world_->pedestrian_, simulation->mode_, simulation->world_->worldType_, simulation->world_->width_, simulation->world_->height_, simulation->K_, simulation->probabilities_);

    if (simulation->mode_ == INTERACTIVE_MODE)
    {
        for (size_t i = 0; i < simulation->numReplications_; i++)
        {
            printf("Cislo aktualnej replikacie: %d\n", i + 1);
            printf("Celkovy pocet replikacii: %d\n", simulation->numReplications_);
            for (size_t j = 0; j < simulation->K_; j++)
            {
                Sleep(1500);
                print_world(simulation->world_);
                move_pedestrian(simulation->world_, simulation->probabilities_);

                fflush(stdin);
            }
        
        initialize_position(simulation->world_);
        initialize_world(simulation->world_, simulation->world_->pedestrian_, simulation->mode_, simulation->world_->worldType_, simulation->world_->width_, simulation->world_->height_, simulation->K_, simulation->probabilities_);
        }
    } else if (simulation->mode_ == SUMMARY_MODE_WITHOUT_K || simulation->mode_ == SUMMARY_MODE_WITH_K) {
        Sleep(1500);
        print_world_summary(simulation->world_, simulation->mode_);
    }
    
    save_simulation_results(simulation, "output.txt");

    free_world(simulation->world_);
    free_pedestrian(simulation->world_->pedestrian_);
}

void save_simulation_results(Simulation* simulation, const char* fileName) {

}


float* choose_probabilities() {
    printf("Zadajte pravdepodobnosti pre pohyb chodca (Pravdepodobnost vsetkych 4 svetovych stran sa musia rovnat 1!):\n");
    
    float* array = (float*)malloc(4 * sizeof(float));
    do {
        printf("1. Pohyb dolava: ");
        scanf("%f", &array[0]);
    
        printf("1. Pohyb doprava: ");
        scanf("%f", &array[1]);

        printf("1. Pohyb hore: ");
        scanf("%f", &array[2]);

        printf("1. Pohyb dole: ");
        scanf("%f", &array[3]);

        if (array[0] + array[1] + array[2] + array[3] != 1.0f) {
            printf("Chyba: Sucet pravdepodobnosti musi byt 1. Skuste to znova.\n");
        }
    } while (array[0] + array[1] + array[2] + array[3] != 1.0f);
    
    return array;
}

int choose_number_of_replications() {
    printf("Zadajte pocet replikacii: ");
    int numReplications;
    scanf("%d", &numReplications);

    return numReplications;
}

void choose_world_type(World* world) {
    printf("Vyberte si typ sveta:\n");
    printf("1. Svet bez prekazok\n");
    printf("2. Svet s prekazkami\n");
    int result;
    scanf("%d", &result);

    switch (result)
    {
    case 1:
        get_size(world);
        world->worldType_ = WORLD_EMPTY;
        break;
    
    case 2:
        choose_world_with_obstacles(world);
        break;
    
    default:
        world->worldType_ = WORLD_EMPTY;
        break;
    }
}

void choose_world_with_obstacles(World* world) {
    printf("Vyber si, ako chces vytvorit svet:\n");
    printf("1. Nahodnou generaciou sveta\n");
    printf("2. Nacitanim zo suboru\n");

    int result;
    scanf("%d", &result);
    switch (result)
    {
    case 1:
        get_size(world);
        generate_world(world);
        world->worldType_ = WORLD_OBSTACLES_GENERATED;
        break;

    case 2:
        world->inputFileName_ = choose_input_file();
        read_world_from_file(world);
        world->worldType_ = WORLD_OBSTACLES_FILE;
        break;
    
    default:
        break;
    }
}

void get_size(World* world) {
    printf("Zadaj pocet riadkov: ");
    scanf("%d", &world->width_);
    printf("Zadaj pocet stlpcov: ");
    scanf("%d", &world->height_);
}

char* choose_input_file() {
    printf("Zadajte nazov pre vstupny subor: ");
    char* fileName = malloc(100 * sizeof(char));
    scanf("%99s", fileName);

    return fileName;
}

char* choose_output_file() {
    printf("Zadajte nazov pre vystupny subor: ");
    char* fileName;
    scanf("%s", &fileName);

    return fileName;
}

bool choose_player_mode() {
    printf("Zvolte rezim pre vasu simulaciu:\n");
    printf("1. Rezim pre jedneho hraca\n");
    printf("2. Rezim pre viac hracov (nefunkcny)\n");
    int result;
    scanf("%d", &result);
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
    printf("Napiste maximalny pocet krokov: ");
    int numberOfSteps;
    scanf("%d", &numberOfSteps);

    return numberOfSteps;
}

SimulationMode choose_mode(Simulation* sim) {
    printf("Zvolte si mod pre simulaciu:\n");
    printf("1. Interaktivny\n");
    printf("2. Sumarny s priemernym poctom posunov\n");
    printf("3. Sumarny s pravdepodobnostou dosiahnutia stredu s najviac K posunmi\n");
    int choice;
    scanf("%d", &choice);
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
    if (simulation)
    {
        free(simulation);
    }
}