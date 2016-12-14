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
	stream << "MSVC++ ";
	switch(_MSC_VER) {
	case 1800: stream << "12.0"; break;
	case 1900: stream << "14.0"; break;
	default: stream << _MSC_VER; break;
	}
#elif defined(__clang__)
	stream << "clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__)
	stream << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#endif

	return stream.str();
}

}
