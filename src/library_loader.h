#ifndef LIBRARY_LOADER_H
#define LIBRARY_LOADER_H

#include "crypto_api.h"

#include <string>

using namespace std;

struct LoadedAlgorithm
{
    void* library_handle = nullptr;
    string algorithm_name;

    GetAlgorithmInfoFunc get_algorithm_info = nullptr;
    GetOutputSizeFunc get_output_size = nullptr;
    CryptFunc encrypt = nullptr;
    CryptFunc decrypt = nullptr;
};

bool is_algorithm_supported(const string& algorithm_name);
string get_library_file_name(const string& algorithm_name);

bool load_algorithm_library(const string& algorithm_name, LoadedAlgorithm& algorithm, string& error_message);
void unload_algorithm_library(LoadedAlgorithm& algorithm);

#endif
