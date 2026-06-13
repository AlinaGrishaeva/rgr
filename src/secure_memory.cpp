#include "secure_memory.h"

using namespace std;

void secure_clear(void* data, size_t size)
{
    if (data == nullptr || size == 0)
    {
        return;
    }

    volatile uint8_t* byte_data = static_cast<volatile uint8_t*>(data);

    for (size_t i = 0; i < size; ++i)
    {
        byte_data[i] = 0;
    }
}

void secure_clear_vector(vector<uint8_t>& data)
{
    if (data.empty())
    {
        return;
    }

    secure_clear(data.data(), data.size());
    data.clear();
    data.shrink_to_fit();
}
