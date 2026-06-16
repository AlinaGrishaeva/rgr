#include "key_utils.h"

#include <fstream>

using namespace std;

vector<uint8_t> generate_key(size_t key_size, string& error_message)
{
    vector<uint8_t> key(key_size);

    if (key_size == 0)
    {
        error_message = "key size is zero";
        return {};
    }

    ifstream random_file("/dev/urandom", ios::binary);

    if (!random_file)
    {
        error_message = "cannot open random generator";
        return {};
    }

    random_file.read(reinterpret_cast<char*>(key.data()), static_cast<streamsize>(key.size()));

    if (!random_file)
    {
        error_message = "cannot generate key";
        return {};
    }

    return key;
}
