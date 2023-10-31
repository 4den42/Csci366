cmake_minimum_required(VERSION 3.16)
project(lmsm C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_executable(lmsm src/main.c src/lmsm.c src/lmsm.h src/assembler.c src/assembler.h src/repl.c src/repl.h src/firth.c src/firth.h)

add_library(lmsm_lib src/main.c src/lmsm.c src/lmsm.h src/assembler.c src/assembler.h src/repl.c src/repl.h src/firth.c src/firth.h)

add_subdirectory(test)

include_directories(SYSTEM ./lib)