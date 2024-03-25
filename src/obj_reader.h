#pragma once

#include <filesystem>
#include <thread>
#include <charconv>
#include "config.h"
#include "linear_algebra.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
struct Chunk;
class File;
struct Reader;

std::pair<std::vector<float>, std::vector<unsigned int>> merge(std::vector<Chunk> chunks);
void consume_line(std::string_view line, Chunk *chunk);
void read_blocks(File *file, int begin, int end, bool stop_at_eol, Chunk *chunk);
std::pair<std::vector<float>, std::vector<unsigned int>> read_obj(const std::string &obj_path);

// Start parsing parallel at this file size (byte).
const size_t thread_threshhold = 1048576;
// Break up into blocks of this size (byte).
const size_t block_size = 262144;
// Maximum size of a line we account for (byte).
const size_t max_line = 4096;

#else
std::pair<std::vector<float>, std::vector<unsigned int>> read_obj(const std::string &obj_path);
#endif