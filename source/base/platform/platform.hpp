#ifndef SU_BASE_PLATFORM_PLATFORM_HPP
#define SU_BASE_PLATFORM_PLATFORM_HPP

#include <string>

namespace platform {

std::string build() noexcept;

std::string revision() noexcept;
}

#endif
