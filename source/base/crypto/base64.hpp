#pragma once

#include <string>

namespace crypto { namespace base64 {

std::string encode(const uint8_t* data, size_t size);

}}
