#include "file.hpp"
#include <cstring>

namespace file {

Type query_type(std::istream& stream) {
	char header[4];
	stream.read(header, sizeof(header));

	Type type = Type::Unknown;

	if (!strncmp("\037\213", header, 2)) {
		type = Type::GZIP;
	} else if (!strncmp("\211PNG", header, 4)) {
		type = Type::PNG;
	} else if (!strncmp("#?", header, 2)) {
		type = Type::RGBE;
	}

	stream.seekg(0);

	return type;
}

}
