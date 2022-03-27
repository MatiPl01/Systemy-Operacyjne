cmake_minimum_required(VERSION 3.16)
project(debug C)

add_compile_options("-Wall")

set(CMAKE_C_STANDARD 11)

set(LIB_NAME libfilessearch)

add_executable(${PROJECT_NAME} main.c)
add_library(${LIB_NAME} ../library/${LIB_NAME}.c)

target_link_libraries(${PROJECT_NAME} ${LIB_NAME})
