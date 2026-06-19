#include "crypto_api.h"

using namespace std;

const size_t CAESAR_KEY_SIZE = 1;
const int SUCCESS_CODE = 0;
const int ERROR_CODE = 1;

const AlgorithmInfo CAESAR_INFO = {
    "caesar",
    CAESAR_KEY_SIZE
};

extern "C" const AlgorithmInfo* get_algorithm_info()
{
    return &CAESAR_INFO;
}

extern "C" size_t get_output_size(size_t input_size, int operation_type)
{
    (void)operation_type;
    return input_size;
}

bool is_buffer_correct(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    if (key.data == nullptr || key.size != CAESAR_KEY_SIZE)
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

extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output)
{
    try
    {
        if (!is_buffer_correct(key, input, output))
        {
            return ERROR_CODE;
        }

        uint8_t shift = key.data[0];

        for (size_t i = 0; i < input.size; ++i)
        {
            output->data[i] = static_cast<uint8_t>(input.data[i] + shift);
        }

        output->size = input.size;
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

        uint8_t shift = key.data[0];

        for (size_t i = 0; i < input.size; ++i)
        {
            output->data[i] = static_cast<uint8_t>(input.data[i] - shift);
        }

        output->size = input.size;
        return SUCCESS_CODE;
    }
    catch (...)
    {
        return ERROR_CODE;
    }
}
