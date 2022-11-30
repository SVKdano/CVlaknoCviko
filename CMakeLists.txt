cmake_minimum_required(VERSION 3.18)
project(cvikoUkazkove C)

set(CMAKE_C_STANDARD 99)

add_executable(cvikoUkazkove main.c nefunkcne.c)
target_link_libraries(cvikoUkazkove pthread)
