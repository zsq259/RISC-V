cmake_minimum_required(VERSION 3.10.2)
project(RISC_V)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -Ofast")

add_executable(code ${src_dir} src/main.cpp)
#add_executable(code ${src_dir} simple-simulator/main.cpp simple-simulator/memory.cpp simple-simulator/cpu.cpp)