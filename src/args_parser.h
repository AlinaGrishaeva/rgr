#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

#include <string>

using std::string;

struct ProgramOptions
{
    bool show_help = false;
    bool generate_key = false;
    bool write_key = false;

    string algorithm;
    string mode;
    string key_file;
    string input_file;
    string output_file;
    string save_key_file;
};

struct ParseResult
{
    bool success = true;
    string error_message;
    ProgramOptions options;
};

ParseResult parse_arguments(int argc, char* argv[]);

#endif
