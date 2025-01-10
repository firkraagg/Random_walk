cmake_minimum_required(VERSION 3.5.0)
project(CMakelists.txt VERSION 0.1.0 LANGUAGES C)

add_executable(CMakelists.txt Menu.c Pedestrian.c Simulation.c World.c client.c main.c server.c)

include(CTest)
enable_testing()

