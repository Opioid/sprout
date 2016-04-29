#include "string.hpp"
#include "math/print.hpp"
#include <sstream>
#include <iomanip>

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

}
