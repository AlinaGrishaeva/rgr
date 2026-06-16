#ifndef KEY_UTILS_H
#define KEY_UTILS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

vector<uint8_t> generate_key(size_t key_size, string& error_message);

#endif
