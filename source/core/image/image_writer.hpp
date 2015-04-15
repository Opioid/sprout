#pragma once

#include <string>

namespace image {

class Image;

bool write(const std::string& filename, const Image& buffer);

}
