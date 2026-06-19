#include "file_utils.h"

#include <fstream>

using namespace std;

vector<uint8_t> read_binary_file(const string& file_name, string& error_message)
{
    error_message.clear();

    ifstream input_file(file_name, ios::binary);

    if (!input_file)
    {
        error_message = "cannot open file for reading: " + file_name;
        return {};
    }

    input_file.seekg(0, ios::end);
    streampos file_size = input_file.tellg();

    if (file_size < 0)
    {
        error_message = "cannot get file size: " + file_name;
        return {};
    }

    input_file.seekg(0, ios::beg);

    vector<uint8_t> data(static_cast<size_t>(file_size));

    if (!data.empty())
    {
        input_file.read(reinterpret_cast<char*>(data.data()), static_cast<streamsize>(data.size()));
    }

    if (!input_file && !input_file.eof())
    {
        error_message = "cannot read file: " + file_name;
        return {};
    }

    return data;
}

vector<uint8_t> read_binary_stream(istream& input_stream, string& error_message)
{
    error_message.clear();

    vector<uint8_t> data;
    vector<uint8_t> block;

    while (read_binary_block(input_stream, block))
    {
        data.insert(data.end(), block.begin(), block.end());
    }

    if (input_stream.bad())
    {
        error_message = "cannot read input stream";
        return {};
    }

    return data;
}

bool write_binary_file(const string& file_name, const vector<uint8_t>& data, string& error_message)
{
    error_message.clear();

    ofstream output_file(file_name, ios::binary);

    if (!output_file)
    {
        error_message = "cannot open file for writing: " + file_name;
        return false;
    }

    if (!data.empty())
    {
        output_file.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    }

    if (!output_file)
    {
        error_message = "cannot write file: " + file_name;
        return false;
    }

    return true;
}

bool write_binary_stream(ostream& output_stream, const vector<uint8_t>& data, string& error_message)
{
    error_message.clear();

    if (!data.empty())
    {
        output_stream.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    }

    if (!output_stream)
    {
        error_message = "cannot write output stream";
        return false;
    }

    return true;
}

bool read_binary_block(istream& input_stream, vector<uint8_t>& data, size_t block_size)
{
    data.assign(block_size, 0);

    input_stream.read(reinterpret_cast<char*>(data.data()), static_cast<streamsize>(data.size()));
    streamsize read_size = input_stream.gcount();

    if (read_size <= 0)
    {
        data.clear();
        return false;
    }

    data.resize(static_cast<size_t>(read_size));
    return true;
}

bool write_binary_block(ostream& output_stream, const vector<uint8_t>& data, string& error_message)
{
    error_message.clear();

    if (!data.empty())
    {
        output_stream.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    }

    if (!output_stream)
    {
        error_message = "cannot write output data";
        return false;
    }

    return true;
}
