#include "library_loader.h"

#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace std;

struct SupportedAlgorithm
{
    string algorithm_name;
    string library_name;
};

const vector<SupportedAlgorithm> SUPPORTED_ALGORITHMS = {
    {"caesar", "caesar"},
    {"code_word", "code_word"},
    {"cardano", "cardano"},
    {"hill", "hill"},
    {"double_square", "double_square"},
    {"vertical_permutation", "vertical_permutation"}
};

bool is_algorithm_supported(const string& algorithm_name)
{
    for (const SupportedAlgorithm& algorithm : SUPPORTED_ALGORITHMS)
    {
        if (algorithm.algorithm_name == algorithm_name)
        {
            return true;
        }
    }

    return false;
}

string get_library_base_name(const string& algorithm_name)
{
    for (const SupportedAlgorithm& algorithm : SUPPORTED_ALGORITHMS)
    {
        if (algorithm.algorithm_name == algorithm_name)
        {
            return algorithm.library_name;
        }
    }

    return "";
}

string get_library_file_name(const string& algorithm_name)
{
    string library_name = get_library_base_name(algorithm_name);

    if (library_name.empty())
    {
        return "";
    }

#ifdef _WIN32
    return library_name + ".dll";
#else
    return "./lib" + library_name + ".so";
#endif
}

void* open_library(const string& library_file_name)
{
#ifdef _WIN32
    return static_cast<void*>(LoadLibraryA(library_file_name.c_str()));
#else
    return dlopen(library_file_name.c_str(), RTLD_NOW);
#endif
}

void* load_library_symbol(void* library_handle, const char* symbol_name)
{
#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(library_handle), symbol_name));
#else
    return dlsym(library_handle, symbol_name);
#endif
}

void close_library(void* library_handle)
{
    if (library_handle == nullptr)
    {
        return;
    }

#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(library_handle));
#else
    dlclose(library_handle);
#endif
}

bool load_library_functions(LoadedAlgorithm& algorithm, string& error_message)
{
    algorithm.get_algorithm_info = reinterpret_cast<GetAlgorithmInfoFunc>(
        load_library_symbol(algorithm.library_handle, "get_algorithm_info")
    );

    algorithm.get_output_size = reinterpret_cast<GetOutputSizeFunc>(
        load_library_symbol(algorithm.library_handle, "get_output_size")
    );

    algorithm.encrypt = reinterpret_cast<CryptFunc>(
        load_library_symbol(algorithm.library_handle, "encrypt")
    );

    algorithm.decrypt = reinterpret_cast<CryptFunc>(
        load_library_symbol(algorithm.library_handle, "decrypt")
    );

    if (algorithm.get_algorithm_info == nullptr ||
        algorithm.get_output_size == nullptr ||
        algorithm.encrypt == nullptr ||
        algorithm.decrypt == nullptr)
    {
        error_message = "cannot load required functions from library";
        return false;
    }

    return true;
}

bool load_algorithm_library(const string& algorithm_name, LoadedAlgorithm& algorithm, string& error_message)
{
    if (!is_algorithm_supported(algorithm_name))
    {
        error_message = "unsupported algorithm: " + algorithm_name;
        return false;
    }

    string library_file_name = get_library_file_name(algorithm_name);

    algorithm.library_handle = open_library(library_file_name);

    if (algorithm.library_handle == nullptr)
    {
        error_message = "cannot open library: " + library_file_name;
        return false;
    }

    if (!load_library_functions(algorithm, error_message))
    {
        close_library(algorithm.library_handle);
        algorithm.library_handle = nullptr;
        return false;
    }

    const AlgorithmInfo* algorithm_info = algorithm.get_algorithm_info();

    if (algorithm_info == nullptr || algorithm_info->algorithm_name == nullptr)
    {
        close_library(algorithm.library_handle);
        algorithm.library_handle = nullptr;
        error_message = "cannot read algorithm information";
        return false;
    }

    algorithm.algorithm_name = algorithm_info->algorithm_name;
    return true;
}
void unload_algorithm_library(LoadedAlgorithm& algorithm)
{
    close_library(algorithm.library_handle);

    algorithm.library_handle = nullptr;
    algorithm.algorithm_name.clear();
    algorithm.get_algorithm_info = nullptr;
    algorithm.get_output_size = nullptr;
    algorithm.encrypt = nullptr;
    algorithm.decrypt = nullptr;
}
