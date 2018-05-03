#ifndef SU_CORE_FILE_FILE_HPP
#define SU_CORE_FILE_FILE_HPP

#include <iosfwd>

namespace file {

enum class Type {
	Undefined,
	GZIP,
	PNG,
	RGBE,
	SUM
};

Type query_type(std::istream& stream);

}

#endif
