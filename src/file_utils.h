#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

using namespace std;

const size_t FILE_BUFFER_SIZE = 65536;

vector<uint8_t> read_binary_file(const string& file_name, string& error_message);
vector<uint8_t> read_binary_stream(istream& input_stream, string& error_message);

bool write_binary_file(const string& file_name, const vector<uint8_t>& data, string& error_message);
bool write_binary_stream(ostream& output_stream, const vector<uint8_t>& data, string& error_message);

bool read_binary_block(istream& input_stream, vector<uint8_t>& data, size_t block_size = FILE_BUFFER_SIZE);
bool write_binary_block(ostream& output_stream, const vector<uint8_t>& data, string& error_message);

#endif
