#include <cstdint>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <vector>

#include "crypto_api.h"

using namespace std;

using GetAlgorithmInfoFunc = const AlgorithmInfo* (*)();
using GetOutputSizeFunc = size_t (*)(size_t, int);
using CipherFunc = int (*)(ConstBuffer, ConstBuffer, MutBuffer*);

const int SUCCESS_CODE = 0;
const int TEST_SUCCESS_CODE = 0;
const int TEST_ERROR_CODE = 1;

struct LibraryFunctions
{
    void* handle;
    GetAlgorithmInfoFunc get_algorithm_info;
    GetOutputSizeFunc get_output_size;
    CipherFunc encrypt;
    CipherFunc decrypt;
};

struct TestCase
{
    string test_name;
    string library_name;
    vector<uint8_t> input_data;
    vector<uint8_t> key;
};

vector<uint8_t> str_to_bytes(const string& text)
{
    return vector<uint8_t>(text.begin(), text.end());
}

string make_library_path(const string& library_name)
{
    return "./lib" + library_name + ".so";
}

vector<uint8_t> make_long_data(size_t size)
{
    vector<uint8_t> data(size);

    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<uint8_t>(i % 256);
    }

    return data;
}

vector<uint8_t> buffer_to_vector(const uint8_t* data, size_t size)
{
    if (size == 0)
    {
        return {};
    }

    return vector<uint8_t>(data, data + size);
}

template <typename FuncType>
bool load_symbol(void* handle, const char* symbol_name, FuncType* output_function)
{
    dlerror();

    void* symbol = dlsym(handle, symbol_name);
    const char* error = dlerror();

    if (error != nullptr || symbol == nullptr)
    {
        return false;
    }

    *output_function = reinterpret_cast<FuncType>(symbol);
    return true;
}

bool load_library(const string& library_name, LibraryFunctions* functions)
{
    if (functions == nullptr)
    {
        return false;
    }

    functions->handle = nullptr;
    functions->get_algorithm_info = nullptr;
    functions->get_output_size = nullptr;
    functions->encrypt = nullptr;
    functions->decrypt = nullptr;

    string library_path = make_library_path(library_name);
    functions->handle = dlopen(library_path.c_str(), RTLD_LAZY);

    if (functions->handle == nullptr)
    {
        cout << "ОШИБКА: не удалось загрузить библиотеку "
             << library_path << endl;
        return false;
    }

    bool loaded =
        load_symbol(functions->handle, "get_algorithm_info", &functions->get_algorithm_info) &&
        load_symbol(functions->handle, "get_output_size", &functions->get_output_size) &&
        load_symbol(functions->handle, "encrypt", &functions->encrypt) &&
        load_symbol(functions->handle, "decrypt", &functions->decrypt);

    if (!loaded)
    {
        cout << "ОШИБКА: в библиотеке " << library_path
             << " отсутствует одна из обязательных функций" << endl;

        dlclose(functions->handle);
        functions->handle = nullptr;

        return false;
    }

    return true;
}

void unload_library(LibraryFunctions* functions)
{
    if (functions != nullptr && functions->handle != nullptr)
    {
        dlclose(functions->handle);
        functions->handle = nullptr;
    }
}

bool is_algorithm_info_correct(const AlgorithmInfo* info,
                               const string& expected_name,
                               const vector<uint8_t>& key)
{
    if (info == nullptr)
    {
        cout << "ОШИБКА: get_algorithm_info вернула nullptr" << endl;
        return false;
    }

    if (info->algorithm_name == nullptr)
    {
        cout << "ОШИБКА: имя алгоритма равно nullptr" << endl;
        return false;
    }

    if (expected_name != info->algorithm_name)
    {
        cout << "ПРЕДУПРЕЖДЕНИЕ: имя алгоритма в библиотеке: "
             << info->algorithm_name << ", имя библиотеки: "
             << expected_name << endl;
    }

    if (info->key_size != 0 && info->key_size != key.size())
    {
        cout << "ОШИБКА: неверный размер ключа для " << info->algorithm_name
             << ". Ожидалось: " << info->key_size
             << ", передано: " << key.size() << endl;
        return false;
    }

    return true;
}

bool encrypt_data(const LibraryFunctions& functions,
                  ConstBuffer key_buffer,
                  ConstBuffer input_buffer,
                  vector<uint8_t>* encrypted_data)
{
    if (encrypted_data == nullptr)
    {
        return false;
    }

    size_t output_size =
        functions.get_output_size(input_buffer.size, OPERATION_ENCRYPT);

    if (output_size == 0 && input_buffer.size > 0)
    {
        cout << "ОШИБКА: get_output_size для шифрования вернула 0" << endl;
        return false;
    }

    vector<uint8_t> output_buffer_data(output_size);
    MutBuffer output_buffer =
    {
        output_buffer_data.data(),
        output_buffer_data.size()
    };

    int result = functions.encrypt(key_buffer, input_buffer, &output_buffer);

    if (result != SUCCESS_CODE)
    {
        cout << "ОШИБКА: шифрование завершилось с кодом "
             << result << endl;
        return false;
    }

    if (output_buffer.size > output_buffer_data.size())
    {
        cout << "ОШИБКА: encrypt установила размер больше выделенного буфера"
             << endl;
        return false;
    }

    *encrypted_data = buffer_to_vector(output_buffer.data, output_buffer.size);
    return true;
}

bool decrypt_data(const LibraryFunctions& functions,
                  ConstBuffer key_buffer,
                  ConstBuffer encrypted_buffer,
                  vector<uint8_t>* decrypted_data)
{
    if (decrypted_data == nullptr)
    {
        return false;
    }

    size_t output_size =
        functions.get_output_size(encrypted_buffer.size, OPERATION_DECRYPT);

    if (output_size == 0 && encrypted_buffer.size > 0)
    {
        cout << "ОШИБКА: get_output_size для расшифрования вернула 0" << endl;
        return false;
    }

    vector<uint8_t> output_buffer_data(output_size);
    MutBuffer output_buffer =
    {
        output_buffer_data.data(),
        output_buffer_data.size()
    };

    int result = functions.decrypt(key_buffer, encrypted_buffer, &output_buffer);

    if (result != SUCCESS_CODE)
    {
        cout << "ОШИБКА: расшифрование завершилось с кодом "
             << result << endl;
        return false;
    }

    if (output_buffer.size > output_buffer_data.size())
    {
        cout << "ОШИБКА: decrypt установила размер больше выделенного буфера"
             << endl;
        return false;
    }

    *decrypted_data = buffer_to_vector(output_buffer.data, output_buffer.size);
    return true;
}

bool run_cipher_test(const TestCase& test_case)
{
    LibraryFunctions functions;

    if (!load_library(test_case.library_name, &functions))
    {
        cout << "ОШИБКА: " << test_case.test_name
             << " - библиотека не загружена" << endl;
        return false;
    }

    const AlgorithmInfo* algorithm_info = functions.get_algorithm_info();

    if (!is_algorithm_info_correct(algorithm_info,
                                   test_case.library_name,
                                   test_case.key))
    {
        cout << "ОШИБКА: " << test_case.test_name
             << " - некорректные метаданные алгоритма" << endl;

        unload_library(&functions);
        return false;
    }

    ConstBuffer key_buffer =
    {
        test_case.key.data(),
        test_case.key.size()
    };

    ConstBuffer input_buffer =
    {
        test_case.input_data.data(),
        test_case.input_data.size()
    };

    vector<uint8_t> encrypted_data;

    if (!encrypt_data(functions, key_buffer, input_buffer, &encrypted_data))
    {
        cout << "ОШИБКА: " << test_case.test_name
             << " - шифрование не выполнено" << endl;

        unload_library(&functions);
        return false;
    }

    ConstBuffer encrypted_buffer =
    {
        encrypted_data.data(),
        encrypted_data.size()
    };

    vector<uint8_t> decrypted_data;

    if (!decrypt_data(functions, key_buffer, encrypted_buffer, &decrypted_data))
    {
        cout << "ОШИБКА: " << test_case.test_name
             << " - расшифрование не выполнено" << endl;

        unload_library(&functions);
        return false;
    }

    unload_library(&functions);

    if (test_case.input_data != decrypted_data)
    {
        cout << "ОШИБКА: " << test_case.test_name
             << " - расшифрованные данные не совпадают с исходными"
             << endl;
        return false;
    }

    cout << "УСПЕХ: " << test_case.test_name << endl;
    return true;
}

vector<TestCase> make_test_cases()
{
    vector<TestCase> tests =
    {
        {
            "caesar_text",
            "caesar",
            str_to_bytes("Hello"),
            {5}
        },
        {
            "caesar_binary",
            "caesar",
            {0, 1, 2, 3, 4, 5},
            {5}
        },
        {
            "caesar_border_values",
            "caesar",
            {255, 128, 64, 32, 0},
            {5}
        },
        {
            "caesar_empty",
            "caesar",
            {},
            {5}
        },

        {
            "code_word_text",
            "code_word",
            str_to_bytes("Hello"),
            str_to_bytes("CODEWORD")
        },
        {
            "code_word_binary",
            "code_word",
            {0, 1, 2, 3, 4, 5},
            str_to_bytes("CODEWORD")
        },
        {
            "code_word_border_values",
            "code_word",
            {255, 128, 64, 32, 0},
            str_to_bytes("CODEWORD")
        },
        {
            "code_word_long_data",
            "code_word",
            make_long_data(1024),
            str_to_bytes("CODEWORD")
        },

        {
            "cardano_text",
            "cardano",
            str_to_bytes("Hello, World!"),
            {0b10001000, 0b00100010}
        },
        {
            "cardano_block",
            "cardano",
            {0, 1, 2, 3, 4, 5, 6, 7,
             8, 9, 10, 11, 12, 13, 14, 15},
            {0b10001000, 0b00100010}
        },
        {
            "cardano_border_values",
            "cardano",
            {255, 128, 64, 32, 0, 1, 2, 3,
             4, 5, 6, 7, 8, 9, 10, 11},
            {0b10001000, 0b00100010}
        },

        {
            "hill_text",
            "hill",
            str_to_bytes("HELLO"),
            {3, 2, 7, 5}
        },
        {
            "hill_binary",
            "hill",
            {0, 1, 2, 3, 4, 5},
            {3, 2, 7, 5}
        },
        {
            "hill_border_values",
            "hill",
            {255, 128, 64, 32, 0, 1, 2, 3},
            {3, 2, 7, 5}
        },

        {
            "vertical_text",
            "vertical_permutation",
            str_to_bytes("Hello"),
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        },
        {
            "vertical_binary",
            "vertical_permutation",
            {0, 1, 2, 3, 4, 5},
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        },
        {
            "vertical_border_values",
            "vertical_permutation",
            {255, 128, 64, 32, 0},
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        },

        {
            "double_square_text",
            "double_square",
            str_to_bytes("test"),
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        },
        {
            "double_square_binary",
            "double_square",
            {10, 20, 30, 40, 50},
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        },
        {
            "double_square_border_values",
            "double_square",
            {0, 255, 1, 254, 2, 253},
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
        }
    };

    return tests;
}

int main()
{
    cout << "ТЕСТЫ КРИПТОГРАФИЧЕСКИХ АЛГОРИТМОВ" << endl;

    vector<TestCase> tests = make_test_cases();

    int failed_count = 0;

    for (const TestCase& test : tests)
    {
        if (!run_cipher_test(test))
        {
            ++failed_count;
        }
    }

    cout << "ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ" << endl;
    cout << "Пройдено: " << (tests.size() - failed_count) << " из " << tests.size() << endl;

    return failed_count > 0 ? 1 : 0;
}
