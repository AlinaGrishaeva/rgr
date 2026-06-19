#include "crypto_api.h"
 
#include <vector>
#include <cstdint>
#include <algorithm>
 
const size_t CARDANO_KEY_SIZE = 2;
const int SUCCESS_CODE = 0;
const int ERROR_CODE = 1;
const size_t CARDANO_BLOCK_SIZE = 16;
const size_t GRID_SIZE = 4;
const size_t TOTAL_CELLS = 16;
 
const AlgorithmInfo CARDANO_INFO =
{
    "cardano",
    CARDANO_KEY_SIZE
};
 
extern "C" const AlgorithmInfo* get_algorithm_info()
{
    return &CARDANO_INFO;
}
 
extern "C" size_t get_output_size(size_t input_size, int operation_type)
{
    (void)operation_type;
    
    size_t padding = CARDANO_BLOCK_SIZE - (input_size % CARDANO_BLOCK_SIZE);
    if (padding == 0) padding = CARDANO_BLOCK_SIZE;
    
    return input_size + padding;
}
 
bool is_buffer_correct_encrypt(
    ConstBuffer key,
    ConstBuffer input,
    MutBuffer* output)
{
    if (key.data == nullptr || key.size != CARDANO_KEY_SIZE)
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
 
    if (output->size < get_output_size(input.size, OPERATION_ENCRYPT))
    {
        return false;
    }
 
    return true;
}
 
bool is_buffer_correct_decrypt(
    ConstBuffer key,
    ConstBuffer input,
    MutBuffer* output)
{
    if (key.data == nullptr || key.size != CARDANO_KEY_SIZE)
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
 
    if (output->size < input.size)
    {
        return false;
    }
 
    return true;
}
 
uint16_t read_mask(ConstBuffer key)
{
    return
        static_cast<uint16_t>(key.data[0]) |
        (static_cast<uint16_t>(key.data[1]) << 8);
}
 
int rotate_position(int position)
{
    int row = position / 4;
    int col = position % 4;
 
    return col * 4 + (3 - row);
}
 
bool validate_mask(uint16_t mask)
{
    bool used[16] = {false};
 
    int holes = 0;
 
    for (int i = 0; i < 16; ++i)
    {
        if ((mask & (1 << i)) != 0)
        {
            ++holes;
 
            int position = i;
 
            for (int rotation = 0; rotation < 4; ++rotation)
            {
                if (used[position])
                {
                    return false;
                }
 
                used[position] = true;
 
                position = rotate_position(position);
            }
        }
    }
 
    if (holes != 4)
    {
        return false;
    }
 
    for (int i = 0; i < 16; ++i)
    {
        if (!used[i])
        {
            return false;
        }
    }
 
    return true;
}
 
std::vector<uint8_t> add_pkcs7_padding(ConstBuffer input)
{
    size_t padding = CARDANO_BLOCK_SIZE - (input.size % CARDANO_BLOCK_SIZE);
    if (padding == 0) padding = CARDANO_BLOCK_SIZE;
    
    std::vector<uint8_t> padded(input.size + padding);
    
    if (input.size > 0)
    {
        std::copy(input.data, input.data + input.size, padded.data());
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
    
    uint8_t padding_value = output->data[output->size - 1];
    
    if (padding_value < 1 || padding_value > CARDANO_BLOCK_SIZE)
    {
        return false;
    }
    
    for (size_t i = output->size - padding_value; i < output->size; ++i)
    {
        if (output->data[i] != padding_value)
        {
            return false;
        }
    }
    
    output->size -= padding_value;
    return true;
}
 
uint16_t get_rotated_mask(uint16_t base_mask, int rotation)
{
    uint16_t mask = base_mask;
    for (int r = 0; r < rotation; ++r)
    {
        uint16_t rotated = 0;
        for (size_t i = 0; i < TOTAL_CELLS; ++i)
        {
            if (mask & (1 << i))
            {
                int new_pos = rotate_position(i);
                rotated |= (1 << new_pos);
            }
        }
        mask = rotated;
    }
    return mask;
}
 
std::vector<int> get_hole_positions(uint16_t base_mask)
{
    std::vector<int> positions;
    
    for (int rotation = 0; rotation < 4; ++rotation)
    {
        uint16_t mask = get_rotated_mask(base_mask, rotation);
        
        for (size_t i = 0; i < TOTAL_CELLS; ++i)
        {
            if (mask & (1 << i))
            {
                positions.push_back(i);
            }
        }
    }
    
    return positions;
}
 
void cardano_encrypt_block(uint8_t* block, uint16_t base_mask)
{
    std::vector<int> positions = get_hole_positions(base_mask);
    
    uint8_t temp[16];
    
    for (size_t i = 0; i < positions.size(); ++i)
    {
        temp[i] = block[positions[i]];
    }
    
    for (size_t i = 0; i < positions.size(); ++i)
    {
        block[positions[i]] = temp[(i + 1) % positions.size()];
    }
}
 
void cardano_decrypt_block(uint8_t* block, uint16_t base_mask)
{
    std::vector<int> positions = get_hole_positions(base_mask);
    
    uint8_t temp[16];
    
    for (size_t i = 0; i < positions.size(); ++i)
    {
        temp[i] = block[positions[i]];
    }
    
    for (size_t i = 0; i < positions.size(); ++i)
    {
        block[positions[i]] = temp[(i + positions.size() - 1) % positions.size()];
    }
}
 
extern "C" int encrypt(
    ConstBuffer key,
    ConstBuffer input,
    MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct_encrypt(key, input, output))
        {
            return ERROR_CODE;
        }
 
        uint16_t base_mask = read_mask(key);
 
        if (!validate_mask(base_mask))
        {
            return ERROR_CODE;
        }
 
        std::vector<uint8_t> padded = add_pkcs7_padding(input);
        
        std::copy(padded.begin(), padded.end(), output->data);
        
        for (size_t block_start = 0; block_start < padded.size(); block_start += CARDANO_BLOCK_SIZE)
        {
            uint8_t* block = output->data + block_start;
            cardano_encrypt_block(block, base_mask);
        }
        
        output->size = padded.size();
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}
 
extern "C" int decrypt(
    ConstBuffer key,
    ConstBuffer input,
    MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct_decrypt(key, input, output))
        {
            return ERROR_CODE;
        }
 
        uint16_t base_mask = read_mask(key);
 
        if (!validate_mask(base_mask))
        {
            return ERROR_CODE;
        }
 
        if (input.size % CARDANO_BLOCK_SIZE != 0)
        {
            return ERROR_CODE;
        }
 
        std::copy(input.data, input.data + input.size, output->data);
        output->size = input.size;
        
        for (size_t block_start = 0; block_start < input.size; block_start += CARDANO_BLOCK_SIZE)
        {
            uint8_t* block = output->data + block_start;
            cardano_decrypt_block(block, base_mask);
        }
        
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
