#include "file.hpp"
#include <cstring>

namespace file {

Type query_type(std::istream& stream) {
	const char png_header[] { static_cast<char>(0x89), 'P', 'N', 'G' };

	char header[4];

	stream.read(header, sizeof(header));

	Type type = Type::Unknown;

	if (!strncmp("#?", header, 2)) {
		type = Type::RGBE;
	} else if (!strncmp(png_header, header, 4)) {
		type = Type::PNG;
	}

	stream.seekg(0, std::ios_base::beg);

	return type;
}

}
