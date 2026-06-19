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
    src/library_loader.cpp
    src/key_utils.cpp
)

target_include_directories(cryptum PRIVATE src)

if(UNIX)
    target_link_libraries(cryptum PRIVATE dl)
endif()

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

add_library(cardano SHARED
    algorithms/cardano/cardano.cpp
)

target_include_directories(cardano PRIVATE src)

set_target_properties(cardano PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(hill SHARED
    algorithms/hill/hill.cpp
)

target_include_directories(hill PRIVATE src)

set_target_properties(hill PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(double_square SHARED
    algorithms/double_square/double_square.cpp
)

target_include_directories(double_square PRIVATE src)

set_target_properties(double_square PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(vertical_permutation SHARED
    algorithms/vertical_permutation/vertical_permutation.cpp
)

target_include_directories(vertical_permutation PRIVATE src)

set_target_properties(vertical_permutation PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_executable(crypto_tests
    tests/crypto_tests.cpp
    src/library_loader.cpp
)

target_include_directories(crypto_tests PRIVATE src)

if(UNIX)
    target_link_libraries(crypto_tests PRIVATE dl)
endif()

add_dependencies(crypto_tests
    caesar
    code_word
    cardano
    hill
    double_square
    vertical_permutation
)
