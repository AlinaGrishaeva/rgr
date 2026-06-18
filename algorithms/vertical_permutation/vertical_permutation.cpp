#include "crypto_api.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>

using namespace std;

const size_t VERTICAL_KEY_SIZE = 32;
const int SUCCESS_CODE = 0;
const int ERROR_CODE = 1;

const AlgorithmInfo VERTICAL_INFO = {
    "vertical_permutation",
    VERTICAL_KEY_SIZE
};

extern "C" const AlgorithmInfo* get_algorithm_info()
{
    return &VERTICAL_INFO;
}

extern "C" size_t get_output_size(size_t input_size, int operation_type)
{
    (void)operation_type;
    return input_size;
}

bool is_buffer_correct(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    if (key.data == nullptr || key.size != VERTICAL_KEY_SIZE)
    {
        return false;
    }

    if (input.size > 0 && input.data == nullptr)
    {
        return false;
    }

    if (output == nullptr || output->size < input.size)
    {
        return false;
    }

    if (input.size > 0 && output->data == nullptr)
    {
        return false;
    }

    return true;
}

vector<uint8_t> vertical_encrypt(const vector<uint8_t>& data, const vector<uint8_t>& key)
{
    if (key.empty()) return data;
    int cols = key.size();
    int rows = (data.size() + cols - 1) / cols;
    
    vector<vector<uint8_t>> table(rows, vector<uint8_t>(cols, 0));
    for (size_t i = 0; i < data.size(); i++) {
        table[i / cols][i % cols] = data[i];
    }
    
    vector<pair<uint8_t, int>> order;
    for (int i = 0; i < cols; i++) {
        order.push_back({key[i], i});
    }
    sort(order.begin(), order.end());
    
    vector<uint8_t> result;
    for (auto& p : order) {
        int col = p.second;
        for (int row = 0; row < rows; row++) {
            if (table[row][col] != 0) {
                result.push_back(table[row][col]);
            }
        }
    }
    return result;
}

vector<uint8_t> vertical_decrypt(const vector<uint8_t>& data, const vector<uint8_t>& key)
{
    if (key.empty()) return data;
    int cols = key.size();
    int rows = (data.size() + cols - 1) / cols;
    int full_cols = data.size() % cols;
    if (full_cols == 0) full_cols = cols;
    
    vector<int> col_lens(cols, rows);
    for (int i = full_cols; i < cols; i++) {
        col_lens[i] = rows - 1;
    }
    
    vector<pair<uint8_t, int>> order;
    for (int i = 0; i < cols; i++) {
        order.push_back({key[i], i});
    }
    sort(order.begin(), order.end());
    
    vector<vector<uint8_t>> table(rows, vector<uint8_t>(cols, 0));
    size_t pos = 0;
    for (auto& p : order) {
        int col = p.second;
        for (int row = 0; row < col_lens[col]; row++) {
            if (pos < data.size()) {
                table[row][col] = data[pos++];
            }
        }
    }
    
    vector<uint8_t> result;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (table[row][col] != 0) {
                result.push_back(table[row][col]);
            }
        }
    }
    return result;
}

extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct(key, input, output))
        {
            return ERROR_CODE;
        }

        vector<uint8_t> k(key.data, key.data + key.size);
        vector<uint8_t> in(input.data, input.data + input.size);
        vector<uint8_t> out = vertical_encrypt(in, k);

        if (out.size() > output->size)
        {
            return ERROR_CODE;
        }

        memcpy(output->data, out.data(), out.size());
        output->size = out.size();
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}

extern "C" int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct(key, input, output))
        {
            return ERROR_CODE;
        }

        vector<uint8_t> k(key.data, key.data + key.size);
        vector<uint8_t> in(input.data, input.data + input.size);
        vector<uint8_t> out = vertical_decrypt(in, k);

        if (out.size() > output->size)
        {
            return ERROR_CODE;
        }

        memcpy(output->data, out.data(), out.size());
        output->size = out.size();
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}
