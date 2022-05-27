cmake_minimum_required(VERSION 3.16)
project(zad1 C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} main.c)

add_library(random lib/librandom.c)
add_library(print lib/libprint.c)

add_library(common ./characters/common.c)
add_library(elf ./characters/elf.c)
add_library(reindeer ./characters/reindeer.c)
add_library(santa ./characters/santa.c)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads m elf common reindeer santa random print)
