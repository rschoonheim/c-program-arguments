cmake_minimum_required(VERSION 3.10)
project(program-arguments C)

set(CMAKE_C_STANDARD 23)

add_library(
        program-arguments
        includes/program_arguments.h
        src/program_arguments.c
)

include_directories(
        includes
)

add_executable(
        example
        example/main.c
)

target_link_libraries(
        example
        program-arguments
)

