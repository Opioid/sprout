#pragma once

#include <string>

namespace image {

class Buffer;

bool write(const std::string& filename, const Buffer& buffer);

}
