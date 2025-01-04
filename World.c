#include "World.h"

void initialize_world(World* world, Pedestrian* pedestrian, SimulationMode mode, WorldType worldType, int width, int height, int K, float probabilities[4]) {
    world->width_ = width;
    world->height_ = height;
    world->grid_ = (char**)malloc(height * sizeof(char*));
    //world->pedestrian_ = pedestrian;
    world->worldType_ = worldType;
    calculate_center(world);

    double probabilities_double[4];
    for (int i = 0; i < 4; i++) {
        probabilities_double[i] = (double)probabilities[i];
    }

    if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_OBSTACLES_FILE) {
        read_world_from_file(world);
    } else {
        for (size_t i = 0; i < height; i++){
            world->grid_[i] = (char*)malloc(width * sizeof(char));
            if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_OBSTACLES_GENERATED) { 
                    generate_world(world);
            }
            for (size_t j = 0; j < width; j++)
            {
                if (mode == INTERACTIVE_MODE && world->worldType_ == WORLD_EMPTY) {
                    world->grid_[i][j] = '.';
                } else if (mode == SUMMARY_MODE_WITHOUT_K) {
                    world->stepsGrid_ = (int**)malloc(height * sizeof(int*));
                    for (size_t i = 0; i < height; i++) {
                        world->stepsGrid_[i] = (int*)malloc(width * sizeof(int));
                        for (size_t j = 0; j < width; j++) {
                            world->stepsGrid_[i][j] = calculate_expected_steps(j, i, world->midX_, world->midY_, probabilities_double);
                        }
                    }
                } else if (mode == SUMMARY_MODE_WITH_K) {
                    world->probabilityGrid_ = (double**)malloc(height * sizeof(double*));
                    for (size_t i = 0; i < height; i++) {
                        world->probabilityGrid_[i] = (double*)malloc(width * sizeof(double));
                        for (size_t j = 0; j < width; j++) {
                            world->probabilityGrid_[i][j] = calculate_probability_to_center(j, i, K, world->midX_, world->midY_, probabilities_double);
                        }
                    }
                }
            }
        }
    }
    
    if (world->pedestrian_ == NULL && world->worldType_ != WORLD_OBSTACLES_FILE) {
        world->pedestrian_ = (Pedestrian*)malloc(sizeof(Pedestrian));
        initialize_position(world);
    }

    reinitialize_world_pedestrian(world);
}

void print_world(World* world) {
    for (size_t i = 0; i < world->height_; i++) {
        printf("\t");
        for (size_t j = 0; j < world->width_; j++) {
            printf("%c ", world->grid_[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_world_summary(World* world, SimulationMode mode) {
    if (mode == SUMMARY_MODE_WITHOUT_K) {
        for (size_t i = 0; i < world->height_; i++) {
            for (size_t j = 0; j < world->width_; j++) {
                printf("%2d ", world->stepsGrid_[i][j]);
            }
            printf("\n");
        }
    } else if (mode == SUMMARY_MODE_WITH_K) {
        for (size_t i = 0; i < world->height_; i++) {
            for (size_t j = 0; j < world->width_; j++) {
                printf("%5.1f%% ", world->probabilityGrid_[i][j] * 100);
            }
            printf("\n");
        }
    }
    printf("\n");
}

void reinitialize_world_pedestrian(World* world) {
    for (size_t i = 0; i < world->height_; i++)
    {
        for (size_t j = 0; j < world->width_; j++)
        {
            if (world->grid_[i][j] != 'O')
            {
                world->grid_[i][j] = '.';
            }
            if (j == world->pedestrian_->x_ && i == world->pedestrian_->y_)
            {
                world->grid_[i][j] = 'C';
            }
        }
    }
}

void free_world(World* world) {
    if (world->stepsGrid_ != NULL) {
        for (size_t i = 0; i < world->height_; i++) {
            free(world->stepsGrid_[i]);
        }

        free(world->stepsGrid_);
    }

    if (world->probabilityGrid_ != NULL) {
        for (size_t i = 0; i < world->height_; i++) {
            free(world->probabilityGrid_[i]);
        }

        free(world->probabilityGrid_);
    }

    free(world->outputFileName_);
}

int read_world_from_file(World* world) {
    FILE* fptr = fopen(world->inputFileName_, "r");
    world->pedestrian_ = (Pedestrian*)malloc(sizeof(Pedestrian));

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

    // initialize_position(world);
    // world->grid_[world->pedestrian_->y_][world->pedestrian_->x_] = 'C';
}

const char* world_type_to_string(WorldType worldType) {
    switch (worldType) {
        case WORLD_EMPTY: return "WORLD_EMPTY";
        case WORLD_OBSTACLES_FILE: return "WORLD_OBSTACLES_FILE";
        case WORLD_OBSTACLES_GENERATED: return "WORLD_OBSTACLES_GENERATED";
        default: return "WORLD_UNKNOWN";
    }
}

double calculate_expected_steps(int x, int y, int midX, int midY, double probabilities[4]) {
    int dx = abs(x - midX);
    int dy = abs(y - midY);

    double expected_horizontal = (dx > 0) ? dx / (probabilities[2] + probabilities[3]) : 0.0;
    double expected_vertical = (dy > 0) ? dy / (probabilities[0] + probabilities[1]) : 0.0;

    return expected_horizontal + expected_vertical;
}

double calculate_probability_to_center(int x, int y, int K, int midX, int midY, double probabilities[4]) {
    if (x == midX && y == midY) {
        return 1.0;
    }
    if (K == 0) {
        return 0.0;
    }

    double probability = 0.0;

    if (y > 0) {
        probability += probabilities[0] * calculate_probability_to_center(x, y - 1, K - 1, midX, midY, probabilities);
    }

    if (y < midY * 2) {
        probability += probabilities[1] * calculate_probability_to_center(x, y + 1, K - 1, midX, midY, probabilities);
    }

    if (x > 0) {
        probability += probabilities[2] * calculate_probability_to_center(x - 1, y, K - 1, midX, midY, probabilities);
    }

    if (x < midX * 2) {
        probability += probabilities[3] * calculate_probability_to_center(x + 1, y, K - 1, midX, midY, probabilities);
    }

    return probability;
}


void calculate_center(World* world) {
    world->midX_ = world->width_ / 2;
    world->midY_ = world->height_ / 2;
}