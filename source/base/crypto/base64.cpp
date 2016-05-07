#include "base64.hpp"
#include <sstream>

namespace crypto { namespace base64 {

static constexpr char base64_rfc4648_alphabet[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

std::string encode(const uint8_t* data, size_t size) {
	std::ostringstream result;
	int i = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (size--) {
		char_array_3[i++] = *(data++);

		if (3 == i) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (uint32_t j = 0; j < 4; ++j) {
				result << base64_rfc4648_alphabet[char_array_4[j]];
			}

			i = 0;
		}
	}

	if (i) {
		for (uint32_t j = i; j < 3; ++j) {
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (uint32_t j = 0; j < i + 1; ++j) {
			result << base64_rfc4648_alphabet[char_array_4[j]];
		}

		while (i++ < 3) {
			result << '=';
		}
	}

	return result.str();
}

}}
