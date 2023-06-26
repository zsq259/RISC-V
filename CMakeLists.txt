cmake_minimum_required(VERSION 3.10.2)
project(RISC_V)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -Ofast")

add_executable(code ${src_dir} src/main.cpp src/memory.cpp src/cpu.cpp)