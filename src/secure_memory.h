#ifndef SECURE_MEMORY_H
#define SECURE_MEMORY_H

#include <cstddef>
#include <cstdint>
#include <vector>

using namespace std;

void secure_clear(void* data, size_t size);
void secure_clear_vector(vector<uint8_t>& data);

#endif
