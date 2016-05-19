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
	text.erase(std::remove_if(text.begin(), text.end(), std::isspace), text.end());
}

}
