#ifndef CRYPTO_API_H
#define CRYPTO_API_H

#include <cstddef>
#include <cstdint>

enum OperationType
{
    OPERATION_ENCRYPT = 1,
    OPERATION_DECRYPT = 2
};

struct ConstBuffer
{
    const uint8_t* data;
    size_t size;
};

struct MutBuffer
{
    uint8_t* data;
    size_t size;
};

struct AlgorithmInfo
{
    const char* algorithm_name;
    size_t key_size;
};

typedef const AlgorithmInfo* (*GetAlgorithmInfoFunc)();
typedef size_t (*GetOutputSizeFunc)(size_t input_size, int operation_type);
typedef int (*CryptFunc)(ConstBuffer key, ConstBuffer input, MutBuffer* output);

#endif
