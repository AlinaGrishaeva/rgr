#include "crypto_api.h"
#include <vector>
#include <cstring>
 
const size_t HILL_KEY_SIZE = 4;
const int SUCCESS_CODE = 0;
const int ERROR_CODE = 1;
const size_t HILL_BLOCK_SIZE = 2;
 
const AlgorithmInfo HILL_INFO =
{
    "hill",
    HILL_KEY_SIZE
};
 
extern "C" const AlgorithmInfo* get_algorithm_info()
{
    return &HILL_INFO;
}
 
extern "C" size_t get_output_size(size_t input_size, int operation_type)
{
    if (operation_type == OPERATION_ENCRYPT)
    {
        size_t padding = HILL_BLOCK_SIZE - (input_size % HILL_BLOCK_SIZE);
        
        if (padding == 0)
        {
            padding = HILL_BLOCK_SIZE;
        }
        
        return input_size + padding;
    }
    else
    {
        return input_size;
    }
}
 
bool is_buffer_correct(ConstBuffer key,
                       ConstBuffer input,
                       MutBuffer* output)
{
    if (key.data == nullptr || key.size != HILL_KEY_SIZE)
    {
        return false;
    }
 
    if (input.size > 0 && input.data == nullptr)
    {
        return false;
    }
 
    if (output == nullptr)
    {
        return false;
    }
 
    if (input.size > 0 && output->data == nullptr)
    {
        return false;
    }
 
    return true;
}
 
int gcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
 
    return a;
}
 
bool is_key_valid(ConstBuffer key)
{
    int a = key.data[0];
    int b = key.data[1];
    int c = key.data[2];
    int d = key.data[3];
 
    int determinant = a * d - b * c;
 
    if (determinant < 0)
    {
        determinant = -determinant;
    }
 
    return gcd(determinant, 256) == 1;
}
 
int find_inverse_mod_256(int value)
{
    value %= 256;
 
    if (value < 0)
    {
        value += 256;
    }
 
    for (int i = 1; i < 256; ++i)
    {
        if ((value * i) % 256 == 1)
        {
            return i;
        }
    }
 
    return -1;
}
 
uint8_t normalize_byte(int value)
{
    value %= 256;
 
    if (value < 0)
    {
        value += 256;
    }
 
    return static_cast<uint8_t>(value);
}
 
std::vector<uint8_t> add_pkcs7_padding(ConstBuffer input)
{
    size_t padding = HILL_BLOCK_SIZE - (input.size % HILL_BLOCK_SIZE);
    
    if (padding == 0)
    {
        padding = HILL_BLOCK_SIZE;
    }
    
    std::vector<uint8_t> padded(input.size + padding);
    
    if (input.size > 0)
    {
        memcpy(padded.data(), input.data, input.size);
    }
    
    for (size_t i = 0; i < padding; ++i)
    {
        padded[input.size + i] = static_cast<uint8_t>(padding);
    }
    
    return padded;
}
 
bool remove_pkcs7_padding(MutBuffer* output)
{
    if (output->size == 0)
    {
        return true;
    }
    
    uint8_t padding = output->data[output->size - 1];
    
    if (padding < 1 || padding > HILL_BLOCK_SIZE)
    {
        return false;
    }
    
    for (size_t i = output->size - padding; i < output->size; ++i)
    {
        if (output->data[i] != padding)
        {
            return false;
        }
    }
    
    output->size -= padding;
    
    return true;
}
 
extern "C" int encrypt(ConstBuffer key,
                       ConstBuffer input,
                       MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct(key, input, output))
        {
            return ERROR_CODE;
        }
        
        if (output->size < get_output_size(input.size, OPERATION_ENCRYPT))
        {
            return ERROR_CODE;
        }
 
        if (!is_key_valid(key))
        {
            return ERROR_CODE;
        }
 
        std::vector<uint8_t> padded = add_pkcs7_padding(input);
        
        int a = key.data[0];
        int b = key.data[1];
        int c = key.data[2];
        int d = key.data[3];
 
        for (size_t i = 0; i < padded.size(); i += HILL_BLOCK_SIZE)
        {
            uint8_t x1 = padded[i];
            uint8_t x2 = padded[i + 1];
 
            output->data[i] = normalize_byte(a * x1 + b * x2);
            output->data[i + 1] = normalize_byte(c * x1 + d * x2);
        }
 
        output->size = padded.size();
 
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}
 
extern "C" int decrypt(ConstBuffer key,
                       ConstBuffer input,
                       MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct(key, input, output))
        {
            return ERROR_CODE;
        }
        
        if (output->size < input.size)
        {
            return ERROR_CODE;
        }
 
        if (!is_key_valid(key))
        {
            return ERROR_CODE;
        }
 
        if (input.size % HILL_BLOCK_SIZE != 0)
        {
            return ERROR_CODE;
        }
 
        int a = key.data[0];
        int b = key.data[1];
        int c = key.data[2];
        int d = key.data[3];
 
        int determinant = a * d - b * c;
 
        int inverse_det = find_inverse_mod_256(determinant);
 
        if (inverse_det == -1)
        {
            return ERROR_CODE;
        }
 
        int ia = d * inverse_det;
        int ib = -b * inverse_det;
        int ic = -c * inverse_det;
        int id = a * inverse_det;
 
        for (size_t i = 0; i < input.size; i += HILL_BLOCK_SIZE)
        {
            uint8_t y1 = input.data[i];
            uint8_t y2 = input.data[i + 1];
 
            output->data[i] = normalize_byte(ia * y1 + ib * y2);
            output->data[i + 1] = normalize_byte(ic * y1 + id * y2);
        }
 
        output->size = input.size;
        
        if (!remove_pkcs7_padding(output))
        {
            return ERROR_CODE;
        }
 
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}
