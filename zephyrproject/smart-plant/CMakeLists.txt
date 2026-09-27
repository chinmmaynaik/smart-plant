cmake_minimum_required(VERSION 3.28.0)

find_package(Zephyr)
project(smart-plant)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE 
    src/main.c
    src/environment_sensor.c
    src/soil_sensor.c    
)