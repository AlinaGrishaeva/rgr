#include "crypto_api.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>

using namespace std;

const size_t DOUBLE_KEY_SIZE = 32;
const int SUCCESS_CODE = 0;
const int ERROR_CODE = 1;

const AlgorithmInfo DOUBLE_SQUARE_INFO = {
    "double_square",
    DOUBLE_KEY_SIZE
};

extern "C" const AlgorithmInfo* get_algorithm_info()
{
    return &DOUBLE_SQUARE_INFO;
}

extern "C" size_t get_output_size(size_t input_size, int operation_type)
{
    (void)operation_type;
    
    if (input_size % 2 != 0) {
        return input_size + 1;
    }
    
    return input_size;
}

bool is_buffer_correct(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    if (key.data == nullptr || key.size != DOUBLE_KEY_SIZE)
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

const int SQUARE_SIZE = 16;

vector<vector<uint8_t>> make_square(const vector<uint8_t>& key)
{
    vector<vector<uint8_t>> square(SQUARE_SIZE, vector<uint8_t>(SQUARE_SIZE, 0));
    vector<uint8_t> used;
    
    for (uint8_t c : key) {
        if (find(used.begin(), used.end(), c) == used.end()) {
            used.push_back(c);
        }
    }
    
    for (int c = 0; c < 256; c++) {
        if (find(used.begin(), used.end(), (uint8_t)c) == used.end()) {
            used.push_back((uint8_t)c);
        }
    }
    
    int idx = 0;
    for (int i = 0; i < SQUARE_SIZE; i++) {
        for (int j = 0; j < SQUARE_SIZE; j++) {
            if (idx < (int)used.size()) {
                square[i][j] = used[idx++];
            }
        }
    }
    return square;
}

void find_pos(const vector<vector<uint8_t>>& sq, uint8_t c, int& r, int& col)
{
    for (int i = 0; i < SQUARE_SIZE; i++) {
        for (int j = 0; j < SQUARE_SIZE; j++) {
            if (sq[i][j] == c) {
                r = i;
                col = j;
                return;
            }
        }
    }
    r = 0;
    col = 0;
}

vector<uint8_t> double_encrypt(const vector<uint8_t>& data, const vector<uint8_t>& k1, const vector<uint8_t>& k2)
{
    auto sq1 = make_square(k1);
    auto sq2 = make_square(k2);
    
    vector<uint8_t> result;
    result.reserve(data.size());

    size_t i = 0;
    for (; i + 1 < data.size(); i += 2) {
        int r1, c1, r2, c2;
        find_pos(sq1, data[i], r1, c1);
        find_pos(sq2, data[i + 1], r2, c2);
        result.push_back(sq1[r1][c2]);
        result.push_back(sq2[r2][c1]);
    }

    if (i < data.size()) {
        int r1, c1, r2, c2;
        find_pos(sq1, data[i], r1, c1);
        find_pos(sq2, data[i], r2, c2);
        result.push_back(sq1[r1][c2]);
        result.push_back(sq2[r2][c1]);
    }

    return result;
}

vector<uint8_t> double_decrypt(const vector<uint8_t>& cipher, const vector<uint8_t>& k1, const vector<uint8_t>& k2)
{
    auto sq1 = make_square(k1);
    auto sq2 = make_square(k2);

    if (cipher.size() % 2 != 0) {
        return {};
    }
    
    vector<uint8_t> result;
    result.reserve(cipher.size());

    for (size_t i = 0; i < cipher.size(); i += 2) {
        int r1, c2, r2, c1;
        find_pos(sq1, cipher[i], r1, c2);
        find_pos(sq2, cipher[i + 1], r2, c1);
        result.push_back(sq1[r1][c1]);
        result.push_back(sq2[r2][c2]);
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
        vector<uint8_t> out = double_encrypt(in, k, k);

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
        vector<uint8_t> out = double_decrypt(in, k, k);

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
