#include <iostream>
#include <string>

void print_help()
{
    std::cout << "Multi-Algo Cryptotool\n";
    std::cout << "Usage:\n";
    std::cout << "  cryptum --help\n";
    std::cout << "  cryptum -a <algorithm> -m generate-key -s <key_file>\n";
    std::cout << "  cryptum -a <algorithm> -m encrypt -k <key_file> -i <input_file> -o <output_file>\n";
    std::cout << "  cryptum -a <algorithm> -m decrypt -k <key_file> -i <input_file> -o <output_file>\n\n";

    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show help\n";
    std::cout << "  -a, --algorithm         Select encryption algorithm\n";
    std::cout << "  -m, --mode              Select mode: encrypt, decrypt, generate-key\n";
    std::cout << "  -k, --key               Read key from binary file\n";
    std::cout << "  -i, --input             Read input data from binary file\n";
    std::cout << "  -o, --output            Write result to binary file\n";
    std::cout << "  -g, --generate-key      Generate key for encryption\n";
    std::cout << "  -s, --save-key          Save generated key to binary file\n";
    std::cout << "  -w, --write-key         Write generated key to standard output\n\n";

    std::cout << "Supported algorithms:\n";
    std::cout << "  caesar\n";
    std::cout << "  xor_cipher\n";
}

bool has_help_flag(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string argument = argv[i];

        if (argument == "-h" || argument == "--help")
        {
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 1 || has_help_flag(argc, argv))
        {
            print_help();
            return 0;
        }

        std::cout << "Error: this mode is not implemented yet.\n";
        std::cout << "Use --help to see available options.\n";
        return 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
