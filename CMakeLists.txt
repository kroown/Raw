cmake_minimum_required(VERSION 3.20)
project(rawc LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rawc
  src/main.cpp
  src/lexer.cpp
  src/parser.cpp
  src/codegen.cpp
)

target_include_directories(rawc PRIVATE src)
