#include "Pedestrian.h"

void free_pedestrian(Pedestrian* pedestrian) {
    if (pedestrian != NULL)
    {
        free(pedestrian);
    }
}

void move_pedestrian(World* world, float probabilities[4]) {
    float rand_number = (float)rand() / RAND_MAX;
    int oldX = world->pedestrian_->x_;
    int oldY = world->pedestrian_->y_;

    if (rand_number < probabilities[0])
    {
        world->pedestrian_->x_ = (world->pedestrian_->x_ - 1 + world->width_) % world->width_;
    } else if (rand_number < probabilities[0] + probabilities[1]) {
        world->pedestrian_->x_ = (world->pedestrian_->x_ + 1 ) % world->width_;
    } else if (rand_number < probabilities[0] + probabilities[1] + probabilities[2])
    {
        world->pedestrian_->y_ = (world->pedestrian_->y_ - 1 + world->height_) % world->height_;
    } else {
        world->pedestrian_->y_ = (world->pedestrian_->y_ + 1) % world->height_;
    }

    if (world->grid_[world->pedestrian_->y_][world->pedestrian_->x_] == 'O') {
        world->pedestrian_->x_ = oldX;
        world->pedestrian_->y_ = oldY;
        move_pedestrian(world, probabilities);
    } else {
        world->grid_[oldY][oldX] = '.';
        world->grid_[world->pedestrian_->y_][world->pedestrian_->x_] = 'C';
    }
}

void generate_random_coordinates(World* world, int* x, int* y) {
    do {
        *x = rand() % world->width_;
        *y = rand() % world->height_;
    } while ((*x == world->midX_ && *y == world->midY_) || (world->grid_[*y][*x] != '.'));
}

void initialize_position(World* world) {
    generate_random_coordinates(world, &world->pedestrian_->startX_, &world->pedestrian_->startY_);
    world->pedestrian_->x_ = world->pedestrian_->startX_;
    world->pedestrian_->y_ = world->pedestrian_->startY_;
    //reinitialize_world_pedestrian(world);
}

void starting_position(World* world, int* x, int* y) {
    *x = world->pedestrian_->startX_;
    *y = world->pedestrian_->startY_;
    reinitialize_world_pedestrian(world);
}