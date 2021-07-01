#pragma once

#include "../runtime/memory_handle.h"
#include <string>


void free_file_binary(binary_file_handle_t& binary_file_to_free);
void read_file_binary(binary_file_handle_t& mem_to_read_to, const char* file_path);
std::string read_file_string(const char* file_path);
void free_image(bitmap_handle_t& image_handle);
void read_image(bitmap_handle_t& image_handle, const char* image_file_path);