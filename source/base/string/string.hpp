#ifndef SU_BASE_STRING_STRING_HPP
#define SU_BASE_STRING_STRING_HPP

#include <iomanip>
#include <sstream>
#include <string>
#include "math/print.hpp"

namespace string {

template <typename T>
static std::string to_string(T value, int num_padded_zeros = 0) noexcept {
    std::ostringstream stream;

    if (num_padded_zeros) {
        stream << std::setw(num_padded_zeros) << std::setfill('0');
    }

    stream << value;
    return stream.str();
}

bool is_space(char c) noexcept;

void trim(std::string& text) noexcept;

std::string_view parent_directory(std::string_view filename) noexcept;

std::string_view suffix(std::string_view filename) noexcept;

std::string_view presuffix(std::string_view filename) noexcept;

std::string extract_filename(std::string const& filename) noexcept;

std::string print_bytes(size_t num_bytes) noexcept;

}  // namespace string

#endif
