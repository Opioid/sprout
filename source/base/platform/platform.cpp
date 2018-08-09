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
#ifndef NDEBUG
    stream << "Debug ";
#else
    stream << "Release ";
#endif

#ifdef SU_DEBUG
    stream << "(NaN) ";
#endif

    // Compiler
#ifdef _MSC_VER
    int const major = _MSC_VER / 100;
    int const minor = _MSC_VER - major * 100;
    stream << "MSVC++ " << major << "." << minor;
#elif defined(__clang__)
    stream << "clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__)
    stream << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#endif

    return stream.str();
}

}  // namespace platform
