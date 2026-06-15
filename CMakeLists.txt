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

add_library(caesar SHARED
    algorithms/caesar/caesar.cpp
)

target_include_directories(caesar PRIVATE src)

set_target_properties(caesar PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(code_word SHARED
    algorithms/code_word/code_word.cpp
)

target_include_directories(code_word PRIVATE src)

set_target_properties(code_word PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
