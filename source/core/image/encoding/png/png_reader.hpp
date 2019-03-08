#ifndef SU_CORE_IMAGE_ENCODING_PNG_READER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_READER_HPP

#include <cstdint>
#include <iosfwd>
#include "image/channels.hpp"

namespace image {

class Image;

namespace encoding::png {

class Reader {
  public:
    static Image* read(std::istream& stream, Channels channels, int32_t num_elements, bool swap_xy,
                       bool invert);
};

}  // namespace encoding::png
}  // namespace image

#endif
