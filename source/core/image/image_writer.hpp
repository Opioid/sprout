#pragma once

#include "typed_image.hpp"
#include <string>

namespace image {

class Image;

bool write(const std::string& filename, const Image_float_4& buffer);

}
