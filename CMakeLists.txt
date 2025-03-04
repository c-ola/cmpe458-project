# This is the main CMake configuration file
# Make sure CMake is synced whenever you make the changes
cmake_minimum_required(VERSION 3.24)
project(compiler C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
# Add executables when needed: Make sure you specify the path to your .c or .h file
#add_executable(my-mini-compiler include/tokens.h src/lexer.c)
add_executable(parser src/parser.c src/lexer.c)
