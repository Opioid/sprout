#include "string.hpp"
#include "math/print.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace string {

template<typename T>
std::string to_string(T value, int num_padded_zeros) {
	std::ostringstream stream;

	if (num_padded_zeros) {
		stream << std::setw(num_padded_zeros) << std::setfill('0');
	}

	stream << value;
	return stream.str();
}

inline void trim(std::string& text) {
	auto trimmed_end = std::remove_if(text.begin(), text.end(), is_space);

	text.erase(trimmed_end, text.end());
}

inline bool is_space(char c) {
	return std::isspace(static_cast<int>(c)) != 0;
}

}
