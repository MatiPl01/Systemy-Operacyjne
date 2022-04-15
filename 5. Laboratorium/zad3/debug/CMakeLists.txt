cmake_minimum_required(VERSION 3.16)
project(debug C)

set(CMAKE_C_STANDARD 11)

add_executable(main main.c)
add_executable(consumer consumer.c)
add_executable(producer producer.c)
