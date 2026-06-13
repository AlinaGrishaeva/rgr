cmake_minimum_required(VERSION 3.10)

project(multi_algo_cryptotool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(cryptum
    src/main.cpp
    src/args_parser.cpp
    src/file_utils.cpp
    src/secure_memory.cpp
)

target_include_directories(cryptum PRIVATE src)
