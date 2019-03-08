#ifndef SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP

#include <iosfwd>
#include "image/typed_image_fwd.hpp"

namespace image::encoding::rgbe {

class Reader {
  public:
    static Image* read(std::istream& stream);
};

}  // namespace image::encoding::rgbe

#endif
