#pragma once

#include <string>

namespace string {

template<typename T>
std::string to_string(T value, int num_padded_zeros = 0);

// removes spaces from front and back
void trim(std::string& text);

bool is_space(char c);

std::string parent_directory(const std::string& filename);

std::string print_bytes(size_t num_bytes);

}
