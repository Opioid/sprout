#include "file.hpp"
#include "gzip/gzip_read_stream.hpp"
#include <fstream>
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

std::unique_ptr<std::istream> open_read_stream(const std::string& name) {
	auto stream = std::unique_ptr<std::istream>(new std::ifstream(name.c_str(), std::ios::binary));

	Type type = query_type(*stream);

	if (Type::GZIP == type) {
		return std::unique_ptr<std::istream>(new gzip::Read_stream(std::move(stream)));
	}

	return stream;
}

}
