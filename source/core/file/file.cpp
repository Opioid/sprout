#include "file.hpp"
#include <cstring>

namespace file {

Type query_type(std::istream& stream) {
	char header[4];
	stream.read(header, sizeof(header));

	Type type = Type::Undefined;

	if (!strncmp("\037\213", header, 2)) {
		type = Type::GZIP;
	} else if (!strncmp("\211PNG", header, 4)) {
		type = Type::PNG;
	} else if (!strncmp("#?", header, 2)) {
		type = Type::RGBE;
	} else if (!strncmp("SUM\005", header, 4)) {
		type = Type::SUM;
	}

	stream.seekg(0);

	return type;
}

std::string read_text(std::istream& stream) {
	std::string text;

	char buffer[4096];

	while (stream.read(buffer, sizeof(buffer))) {
		text.append(buffer, sizeof(buffer));
	}

	text.append(buffer, stream.gcount());

	return text;
}

}
