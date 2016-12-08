#include "platform.hpp"
#include <sstream>

namespace platform {

std::string build() {
	std::stringstream stream;

	// Architecture
#if defined(__LP64__) || defined(_LP64) || defined(_WIN64)
	stream << "x64 ";
#else
	stream << "x86 ";
#endif

	// Profile
#ifdef _DEBUG
	stream << "Debug ";
#else
	stream << "Release ";
#endif

#ifdef SU_DEBUG
	stream << "(NaN) ";
#endif

	// Compiler
#ifdef _MSC_VER
	std::stringstream version;
	version << _MSC_VER;
	stream << "VS " << (_MSC_VER / 100) << "." << version.str().substr(2, 2);
#elif defined(__clang__)
	stream << "clang " << __clang_major__ << "." << __clang_minor__;
#endif

	return stream.str();
}

}
