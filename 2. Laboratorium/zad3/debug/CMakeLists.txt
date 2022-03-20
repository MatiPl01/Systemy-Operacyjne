cmake_minimum_required(VERSION 3.16)
project(debug C)

add_compile_options("-Wall")

set(CMAKE_C_STANDARD 11)

set(STAT_LIB_NAME liblistdirstat)
set(NFTW_LIB_NAME liblistdirnftw)

add_executable(${PROJECT_NAME} main.c)
add_library(${STAT_LIB_NAME} ../libraries/${STAT_LIB_NAME}.c)
#add_library(${NFTW_LIB_NAME} ../libraries/${NFTW_LIB_NAME}.c)

target_link_libraries(${PROJECT_NAME} ${STAT_LIB_NAME})
#target_link_libraries(${PROJECT_NAME} ${NFTW_LIB_NAME})
