#ifndef SU_BASE_STRING_STRING_HPP
#define SU_BASE_STRING_STRING_HPP

#include "math/print.hpp"

#include <iomanip>
#include <sstream>
#include <string>

namespace string {

template <typename T>
static std::string to_string(T value, int num_padded_zeros = 0) {
    std::ostringstream stream;

    if (num_padded_zeros) {
        stream << std::setw(num_padded_zeros) << std::setfill('0');
    }

    stream << value;
    return stream.str();
}

bool is_json(std::string_view text);

void trim(std::string& text);

std::string_view parent_directory(std::string_view filename);

std::string_view suffix(std::string_view filename);

std::string copy_suffix(std::string const& filename);

std::string_view presuffix(std::string_view filename);

std::string extract_filename(std::string const& filename);

std::string print_bytes(size_t num_bytes);

}  // namespace string

#endif
