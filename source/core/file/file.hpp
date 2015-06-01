#pragma once

#include <istream>
#include <memory>
#include <string>

namespace file {

enum class Type {
	Unknown,
	GZIP,
	PNG,
	RGBE
};

Type query_type(std::istream& stream);

std::unique_ptr<std::istream> open_read_stream(const std::string& name);

//std::istream open_read_stream(const std::string& name);

}
