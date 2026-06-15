#include "args_parser.h"
#include "library_loader.h"

#include <exception>
#include <iostream>

using namespace std;

void print_help()
{
    cout << "Multi-Algo Cryptotool\n";
    cout << "Usage:\n";
    cout << "  cryptum --help\n";
    cout << "  cryptum -a <algorithm> -m generate-key -s <key_file>\n";
    cout << "  cryptum -a <algorithm> -m encrypt -k <key_file> -i <input_file> -o <output_file>\n";
    cout << "  cryptum -a <algorithm> -m decrypt -k <key_file> -i <input_file> -o <output_file>\n\n";

    cout << "Options:\n";
    cout << "  -h, --help              Show help\n";
    cout << "  -a, --algorithm         Select encryption algorithm\n";
    cout << "  -m, --mode              Select mode: encrypt, decrypt, generate-key\n";
    cout << "  -k, --key               Read key from binary file\n";
    cout << "  -i, --input             Read input data from binary file\n";
    cout << "  -o, --output            Write result to binary file\n";
    cout << "  -g, --generate-key      Generate key for encryption\n";
    cout << "  -s, --save-key          Save generated key to binary file\n";
    cout << "  -w, --write-key         Write generated key to standard output\n\n";

    cout << "Supported algorithms:\n";
    cout << "  caesar       - Caesar cipher\n";
    cout << "  code_word    - code word cipher\n";
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

        cout << "Algorithm library loaded.\n";
        cout << "Selected algorithm: " << algorithm_info->algorithm_name << '\n';
        cout << "Key size: " << algorithm_info->key_size << " byte(s)\n";
        cout << "Selected mode: " << result.options.mode << '\n';
        cout << "This mode will be implemented in the next steps.\n";

        unload_algorithm_library(algorithm);
        return 0;
    }
    catch (const exception& error)
    {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
