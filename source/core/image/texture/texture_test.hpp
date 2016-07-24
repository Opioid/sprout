#pragma one

#include <string>

namespace image {

class Image;

namespace texture { namespace testing {

bool is_valid_normal_map(const Image& image, const std::string& filename);

}}}
