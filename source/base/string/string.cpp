#include "string.hpp"

#include <algorithm>
#include <cctype>

namespace string {

bool is_json(std::string_view text) {
    auto const it = text.find_first_not_of(" \t");

    if (std::string::npos != it) {
        return '{' == text[it];
    }

    return false;
}

std::string_view parent_directory(std::string_view filename) {
    if (filename.empty()) {
        return filename;
    }

    size_t const i = filename.find_last_of('/');
    return filename.substr(0, i + 1);
}

std::string_view suffix(std::string_view filename) {
    size_t const i = filename.find_last_of('.');
    return filename.substr(i + 1, std::string::npos);
}

std::string copy_suffix(std::string const& filename) {
    size_t const i = filename.find_last_of('.');
    return filename.substr(i + 1, std::string::npos);
}

std::string_view presuffix(std::string_view filename) {
    size_t const i = filename.find_last_of('.');
    size_t const j = filename.substr(0, i).find_last_of('.') + 1;
    return filename.substr(j, i - j);
}

std::string extract_filename(std::string const& filename) {
    size_t const i = filename.find_last_of('/') + 1;
    return filename.substr(i, filename.find_first_of('.') - i);
}

std::string print_bytes(size_t num_bytes) {
    std::ostringstream stream;

    stream << std::fixed << std::setprecision(2);

    if (num_bytes < 1024) {
        stream << num_bytes;
        stream << " B";
    } else if (num_bytes < 1024 * 1024) {
        stream << float(num_bytes) / 1024.f;
        stream << " KiB";
    } else if (num_bytes < 1024 * 1024 * 1024) {
        stream << float(num_bytes) / (1024.f * 1024.f);
        stream << " MiB";
    } else {
        stream << float(num_bytes) / (1024.f * 1024.f * 1024.f);
        stream << " GiB";
    }

    return stream.str();
}

}  // namespace string
