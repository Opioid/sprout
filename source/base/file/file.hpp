#pragma once

#include <istream>

namespace file {

enum class Type {
	Unknown,
	PNG,
	RGBE
};

Type query_type(std::istream& stream);

}
