#pragma once

#include <istream>
#include <memory>
#include <string>

namespace file {

enum class Type {
	Unknown,
	GZIP,
	PNG,
	RGBE,
	SUM
};

Type query_type(std::istream& stream);

}
