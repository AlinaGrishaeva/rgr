#include "args_parser.h"
#include "file_utils.h"
#include "key_utils.h"
#include "library_loader.h"
#include "secure_memory.h"

#include <exception>
#include <iostream>
#include <vector>

using namespace std;

void print_help()
{
    cout << "Multi-Algo Cryptotool\n";
    cout << "Usage:\n";
    cout << "  cryptum --help\n";
    cout << "  cryptum -a <algorithm> -m generate-key -s <key_file>\n";
    cout << "  cryptum -a <algorithm> -m generate-key -w\n";
    cout << "  cryptum -a <algorithm> -m encrypt -k <key_file> -i <input_file> -o <output_file>\n";
    cout << "  cryptum -a <algorithm> -m encrypt -g -s <key_file> -i <input_file> -o <output_file>\n";
    cout << "  cryptum -a <algorithm> -m decrypt -k <key_file> -i <input_file> -o <output_file>\n";
    cout << "  cat <input_file> | cryptum -a <algorithm> -m encrypt -k <key_file> -o <output_file>\n";
    cout << "  cryptum -a <algorithm> -m decrypt -k <key_file> -i <input_file> > <output_file>\n\n";

    cout << "Options:\n";
    cout << "  -h, --help              Show help\n";
    cout << "  -a, --algorithm         Select encryption algorithm\n";
    cout << "  -m, --mode              Select mode: encrypt, decrypt, generate-key\n";
    cout << "  -k, --key               Read key from binary file or stdin with '-'\n";
    cout << "  -i, --input             Read input data from binary file or stdin with '-'\n";
    cout << "  -o, --output            Write result to binary file or stdout with '-'\n";
    cout << "  -g, --generate-key      Generate key for encryption\n";
    cout << "  -s, --save-key          Save generated key to binary file\n";
    cout << "  -w, --write-key         Write generated key to standard output\n\n";

    cout << "Supported algorithms:\n";
    cout << "  caesar       - Caesar cipher\n";
    cout << "  code_word    - code word cipher\n";
    cout << "  cardano                - Cardano grille cipher\n";
    cout << "  hill                   - Hill cipher\n";
    cout << "  double_square          - double square cipher\n";
    cout << "  vertical_permutation   - vertical permutation cipher\n";
}

bool is_standard_stream_name(const string& file_name)
{
    return file_name.empty() || file_name == "-";
}

bool is_stdin_name(const string& file_name)
{
    return file_name == "-";
}

bool is_stdout_name(const string& file_name)
{
    return file_name.empty() || file_name == "-";
}

int run_generate_key_mode(const ProgramOptions& options, const AlgorithmInfo* algorithm_info)
{
    if (!options.save_key_file.empty() && options.write_key)
    {
        cerr << "Error: choose only one key output method\n";
        return 1;
    }

    string error_message;
    vector<uint8_t> key = generate_key(algorithm_info->key_size, error_message);

    if (key.empty())
    {
        cerr << "Error: " << error_message << '\n';
        return 1;
    }

    if (!options.save_key_file.empty())
    {
        if (!write_binary_file(options.save_key_file, key, error_message))
        {
            secure_clear_vector(key);
            cerr << "Error: " << error_message << '\n';
            return 1;
        }

        cout << "Key generated successfully.\n";
        cout << "Algorithm: " << algorithm_info->algorithm_name << '\n';
        cout << "Key size: " << algorithm_info->key_size << " byte(s)\n";
    }
    else if (options.write_key)
    {
        if (!write_binary_stream(cout, key, error_message))
        {
            secure_clear_vector(key);
            cerr << "Error: " << error_message << '\n';
            return 1;
        }
    }

    secure_clear_vector(key);
    return 0;
}

int get_operation_type(const string& mode)
{
    if (mode == "encrypt")
    {
        return OPERATION_ENCRYPT;
    }

    return OPERATION_DECRYPT;
}

CryptFunc get_crypt_function(const LoadedAlgorithm& algorithm, const string& mode)
{
    if (mode == "encrypt")
    {
        return algorithm.encrypt;
    }

    return algorithm.decrypt;
}

bool are_standard_streams_valid(const ProgramOptions& options)
{
    if (is_stdin_name(options.key_file) && is_standard_stream_name(options.input_file))
    {
        return false;
    }

    if (options.generate_key && options.write_key && is_stdout_name(options.output_file))
    {
        return false;
	}

    return true;
}

bool save_generated_key_if_needed(const ProgramOptions& options, const vector<uint8_t>& key, string& error_message)
{
    if (!options.save_key_file.empty())
    {
        return write_binary_file(options.save_key_file, key, error_message);
    }

    if (options.write_key)
    {
        return write_binary_stream(cout, key, error_message);
    }

    return true;
}

vector<uint8_t> read_key_for_crypt_mode(const ProgramOptions& options, string& error_message)
{
    if (is_stdin_name(options.key_file))
    {
        return read_binary_stream(cin, error_message);
    }

    return read_binary_file(options.key_file, error_message);
}

vector<uint8_t> get_key_for_crypt_mode(const ProgramOptions& options, const AlgorithmInfo* algorithm_info, string& error_message)
{
    if (options.generate_key)
    {
        if (!options.key_file.empty())
        {
            error_message = "choose key file or key generation";
            return {};
        }

        vector<uint8_t> key = generate_key(algorithm_info->key_size, error_message);

        if (key.empty())
        {
            return {};
        }

        if (!save_generated_key_if_needed(options, key, error_message))
        {
            secure_clear_vector(key);
            return {};
        }

        return key;
    }

    return read_key_for_crypt_mode(options, error_message);
}

vector<uint8_t> read_input_data(const ProgramOptions& options, string& error_message)
{
    if (is_standard_stream_name(options.input_file))
    {
        return read_binary_stream(cin, error_message);
    }

    return read_binary_file(options.input_file, error_message);
}

bool write_output_data(const ProgramOptions& options, const vector<uint8_t>& output_data, string& error_message)
{
    if (is_stdout_name(options.output_file))
    {
        return write_binary_stream(cout, output_data, error_message);
    }

    return write_binary_file(options.output_file, output_data, error_message);
}

void print_crypt_success(const ProgramOptions& options, const AlgorithmInfo* algorithm_info)
{
    ostream& message_stream = is_stdout_name(options.output_file) ? cerr : cout;

    message_stream << "Operation completed successfully.\n";
    message_stream << "Algorithm: " << algorithm_info->algorithm_name << '\n';
    message_stream << "Mode: " << options.mode << '\n';
}

int run_crypt_mode(const ProgramOptions& options, const LoadedAlgorithm& algorithm, const AlgorithmInfo* algorithm_info)
{
    if (!are_standard_streams_valid(options))
    {
        cerr << "Error: invalid standard stream usage\n";
        return 1;
    }

    string error_message;

    vector<uint8_t> key = get_key_for_crypt_mode(options, algorithm_info, error_message);

    if (key.empty())
    {
        cerr << "Error: " << error_message << '\n';
        return 1;
    }

    if (key.size() != algorithm_info->key_size)
    {
        secure_clear_vector(key);
        cerr << "Error: incorrect key size\n";
        return 1;
    }

    vector<uint8_t> input_data = read_input_data(options, error_message);

    if (!error_message.empty())
    {
        secure_clear_vector(key);
        cerr << "Error: " << error_message << '\n';
        return 1;
    }

    int operation_type = get_operation_type(options.mode);
    size_t output_size = algorithm.get_output_size(input_data.size(), operation_type);

    vector<uint8_t> output_data(output_size);

    ConstBuffer key_buffer = {key.data(), key.size()};
    ConstBuffer input_buffer = {input_data.data(), input_data.size()};
    MutBuffer output_buffer = {output_data.data(), output_data.size()};

    CryptFunc crypt_function = get_crypt_function(algorithm, options.mode);
    int crypt_result = crypt_function(key_buffer, input_buffer, &output_buffer);

    if (crypt_result != 0)
    {
        secure_clear_vector(key);
        secure_clear_vector(input_data);
        secure_clear_vector(output_data);
        cerr << "Error: cryptographic operation failed\n";
	return 1;
    }

    output_data.resize(output_buffer.size);

    if (!write_output_data(options, output_data, error_message))
    {
        secure_clear_vector(key);
        secure_clear_vector(input_data);
        secure_clear_vector(output_data);
        cerr << "Error: " << error_message << '\n';
        return 1;
    }

    secure_clear_vector(key);
    secure_clear_vector(input_data);
    secure_clear_vector(output_data);

    print_crypt_success(options, algorithm_info);
    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        ParseResult result = parse_arguments(argc, argv);

        if (result.options.show_help)
        {
            print_help();
            return 0;
        }

        if (!result.success)
        {
            cerr << "Error: " << result.error_message << '\n';
            cerr << "Use --help to see available options.\n";
            return 1;
        }

        LoadedAlgorithm algorithm;
        string error_message;

        if (!load_algorithm_library(result.options.algorithm, algorithm, error_message))
        {
            cerr << "Error: " << error_message << '\n';
            return 1;
        }

        const AlgorithmInfo* algorithm_info = algorithm.get_algorithm_info();

        if (result.options.mode == "generate-key")
        {
            int result_code = run_generate_key_mode(result.options, algorithm_info);
            unload_algorithm_library(algorithm);
            return result_code;
        }

        int result_code = run_crypt_mode(result.options, algorithm, algorithm_info);
        unload_algorithm_library(algorithm);
        return result_code;
    }
    catch (const exception& error)
    {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
