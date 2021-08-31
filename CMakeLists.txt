cmake_minimum_required(VERSION 3.12)
project(snekMMO)

add_compile_options(-Wall -Wextra)

file(GLOB_RECURSE SOURCES src/*.c)

add_executable(snekMMO ${SOURCES})
target_link_libraries(snekMMO pthread)
