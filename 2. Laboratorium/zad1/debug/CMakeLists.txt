cmake_minimum_required(VERSION 3.16)
project(debug C)

add_compile_options("-Wall")

set(CMAKE_C_STANDARD 11)

set(LIB_LIB_NAME libcopylib)
set(SYS_LIB_NAME libcopysys)

add_executable(${PROJECT_NAME} main.c)
#add_library(${LIB_LIB_NAME} ../libraries/${LIB_LIB_NAME}.c)
add_library(${SYS_LIB_NAME} ../libraries/${SYS_LIB_NAME}.c)

#target_link_libraries(${PROJECT_NAME} ${LIB_LIB_NAME})
target_link_libraries(${PROJECT_NAME} ${SYS_LIB_NAME})
