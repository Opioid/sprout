#ifndef SU_CORE_IMAGE_TEXTURE_TEST_HPP
#define SU_CORE_IMAGE_TEXTURE_TEST_HPP

#include <string>

namespace image {

class Image;

namespace texture::testing {

bool is_valid_normal_map(const Image const& image, std::string const& filename);
}
}  // namespace image

#endif
