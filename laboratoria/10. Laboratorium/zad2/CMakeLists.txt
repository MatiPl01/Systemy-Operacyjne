cmake_minimum_required(VERSION 3.16)
project(zad1 C)

set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(client client.c)
add_executable(server server.c)

add_library(common lib/libcommon.c)
add_library(cprint lib/libcprint.c)

target_link_libraries(client cprint common)
target_link_libraries(server PRIVATE Threads::Threads m cprint common)
