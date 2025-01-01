#include "World.h"

void initialize_world(World* world, Pedestrian* pedestrian, SimulationMode mode, WorldType worldType, int width, int height, int K) {
    world->width_ = width;
    world->height_ = height;
    world->grid_ = (char**)malloc(height * sizeof(char*));
    world->pedestrian_ = pedestrian;
    world->worldType_ = worldType;
    calculate_center(world);
    for (size_t i = 0; i < height; i++)
    {
        world->grid_[i] = (char*)malloc(width * sizeof(char));
        for (size_t j = 0; j < width; j++)
        {
            if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_EMPTY)
            {
                world->grid_[i][j] = '.';
                if (j == world->pedestrian_->x_ && i == world->pedestrian_->y_)
                {
                    world->grid_[i][j] = 'C';
                }
            } else if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_OBSTACLES_FILE) {
                read_world_from_file(world);
            } else if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_OBSTACLES_GENERATED) { 
                generate_world(world);
            } else if (mode == SUMMARY_MODE_WITHOUT_K) {
                //int distance = calculate_expected_steps(j, i, world->midX_, world->midY_, world->pedestrian_->probabilities_);
                //world->grid_[i][j] = '0' + (distance % 10);
            } else if (mode == SUMMARY_MODE_WITH_K) {
                //double probability = calculate_probability_with_K_steps(j, i, world->midX_, world->midY_, K);
                //world->grid_[i][j] = '0' + (int)(probability * 10);
            }
        }
    }
}

void print_world(World* world) {
    for (size_t i = 0; i < world->height_; i++) {
        for (size_t j = 0; j < world->width_; j++) {
            printf("%c ", world->grid_[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void free_world(World* world) {
    for (size_t i = 0; i < world->height_; i++)
    {
        free(world->grid_[i]);
    }

    free(world->grid_);
    world->grid_ = NULL;
}

int read_world_from_file(World* world) {
    FILE* fptr = fopen(world->inputFileName_, "r");

    if (fptr == NULL) {
        printf("Nebolo mozne otvorit subor.\n");
        return 1;
    }

    int rows = 0, cols = 0;
    char line[256];

    while (fgets(line, sizeof(line), fptr)) {
        if (rows == 0) {
            for (char* token = strtok(line, " \n"); token != NULL; token = strtok(NULL, " \n")) {
                cols++;
            }
        }
        rows++;
    }

    world->width_ = cols;
    world->height_ = rows;
    world->grid_ = (char**)malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        world->grid_[i] = (char*)malloc(cols * sizeof(char));
    }

    rewind(fptr);

    int row = 0;
    while (fgets(line, sizeof(line), fptr)) {
        int col = 0;
        for (char* token = strtok(line, " \n"); token != NULL; token = strtok(NULL, " \n")) {
            world->grid_[row][col] = token[0];
            if(token[0] == 'C') {
                world->pedestrian_->startX_ = col;
                world->pedestrian_->startY_ = row;
                world->pedestrian_->x_ = col;
                world->pedestrian_->y_ = row;
            }
            col++;
        }
        row++;
    }

    fclose(fptr);
    return 0;
}

void generate_world(World* world) {
    world->grid_ = (char**)malloc(world->height_ * sizeof(char*));
    for (int i = 0; i < world->height_; i++) {
        world->grid_[i] = (char*)malloc(world->width_ * sizeof(char));
    }

    for (int i = 0; i < world->height_; i++) {
        for (int j = 0; j < world->width_; j++) {
            double probability = (double)rand() / RAND_MAX;
            if (probability < 0.2 && i != world->midX_ && j != world->midY_) {
                world->grid_[i][j] = 'O'; 
            } else {
                world->grid_[i][j] = '.';
            }
        }
    }

    initialize_position(world);
    world->grid_[world->pedestrian_->y_][world->pedestrian_->x_] = 'C';
}


double calculate_expected_steps(int x, int y, int midX, int midY, double probabilities[4]) {
    // Pravdepodobnosti pohybu v smere hore, dole, vľavo, vpravo
    // [0] = pohyb hore, [1] = pohyb dole, [2] = pohyb vľavo, [3] = pohyb vpravo
    
    // Pre jednoduchosť použijeme Manhattanovu vzdialenosť k stredu a zohľadníme pravdepodobnosti
    double distance = abs(x - midX) + abs(y - midY);
    double expected_steps = 0;
    
    // Očakávaný počet krokov bude ovplyvnený pravdepodobnosťou pre každý smer
    // Týmto spôsobom zahrnieme pravdepodobnosti do výpočtu
    if (x > midX) {  // Pohyb vľavo
        expected_steps += probabilities[2] * (x - midX);
    } else if (x < midX) {  // Pohyb vpravo
        expected_steps += probabilities[3] * (midX - x);
    }
    
    if (y > midY) {  // Pohyb hore
        expected_steps += probabilities[0] * (y - midY);
    } else if (y < midY) {  // Pohyb dole
        expected_steps += probabilities[1] * (midY - y);
    }
    
    // Ak sú pravdepodobnosti rovnaké pre všetky smery, očakávaný počet krokov bude priamo úmerný vzdialenosti
    expected_steps = distance / expected_steps;
    
    return expected_steps;
}

// int calculate_probability_with_K_steps(int x, int y, int midX, int midY, int K) {
//     int distance = calculate_steps_to_center(x, y, midX, midY);
//     if (distance > K) {
//         return 0.0;  
//     }
//     return 1.0 - (double)distance / K;
// }

void calculate_center(World* world) {
    world->midX_ = world->width_ / 2;
    world->midY_ = world->height_ / 2;
}