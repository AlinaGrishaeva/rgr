#include "args_parser.h"

#include <string>

using namespace std;

bool is_help_argument(const string& argument)
{
    return argument == "-h" || argument == "--help";
}

bool is_mode_value_correct(const string& mode)
{
    return mode == "encrypt" || mode == "decrypt" || mode == "generate-key";
}

bool has_next_value(int index, int argc)
{
    return index + 1 < argc;
}

void set_error(ParseResult& result, const string& message)
{
    result.success = false;
    result.error_message = message;
}

bool read_argument_value(int& index, int argc, char* argv[], string& value, ParseResult& result)
{
    if (!has_next_value(index, argc))
    {
        set_error(result, "missing value for argument: " + string(argv[index]));
        return false;
    }

    ++index;
    value = argv[index];
    return true;
}

void validate_options(ParseResult& result)
{
    ProgramOptions& options = result.options;

    if (options.show_help)
    {
        return;
    }

    if (options.algorithm.empty())
    {
        set_error(result, "algorithm is not specified");
        return;
    }

    if (options.mode.empty())
    {
        set_error(result, "mode is not specified");
        return;
    }

    if (!is_mode_value_correct(options.mode))
    {
        set_error(result, "unknown mode: " + options.mode);
        return;
    }

    if (options.mode == "generate-key")
    {
        if (options.save_key_file.empty() && !options.write_key)
        {
            set_error(result, "key output is not specified");
        }

        return;
    }

    if (options.mode == "decrypt" && options.key_file.empty())
    {
        set_error(result, "key file is not specified");
        return;
    }

    if (options.mode == "encrypt" && options.key_file.empty() && !options.generate_key)
    {
        set_error(result, "key file is not specified");
    }
}

ParseResult parse_arguments(int argc, char* argv[])
{
    ParseResult result;

    if (argc == 1)
    {
        result.options.show_help = true;
        return result;
    }

    for (int i = 1; i < argc; ++i)
    {
        string argument = argv[i];

        if (is_help_argument(argument))
        {
            result.options.show_help = true;
            return result;
        }

        if (argument == "-a" || argument == "--algorithm")
        {
            if (!read_argument_value(i, argc, argv, result.options.algorithm, result))
            {
                return result;
            }
        }
        else if (argument == "-m" || argument == "--mode")
        {
            if (!read_argument_value(i, argc, argv, result.options.mode, result))
            {
                return result;
            }
        }
        else if (argument == "-k" || argument == "--key")
        {
            if (!read_argument_value(i, argc, argv, result.options.key_file, result))
            {
                return result;
            }
        }
        else if (argument == "-i" || argument == "--input")
        {
            if (!read_argument_value(i, argc, argv, result.options.input_file, result))
            {
                return result;
            }
        }
        else if (argument == "-o" || argument == "--output")
        {
            if (!read_argument_value(i, argc, argv, result.options.output_file, result))
            {
                return result;
            }
        }
        else if (argument == "-g" || argument == "--generate-key")
        {
            result.options.generate_key = true;
        }
        else if (argument == "-s" || argument == "--save-key")
        {
            if (!read_argument_value(i, argc, argv, result.options.save_key_file, result))
            {
                return result;
            }
        }
        else if (argument == "-w" || argument == "--write-key")
        {
            result.options.write_key = true;
        }
        else
        {
            set_error(result, "unknown argument: " + argument);
            return result;
        }
    }
    validate_options(result);
    return result;
}

