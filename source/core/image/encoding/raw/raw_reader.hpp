#ifndef SU_CORE_IMAGE_ENCODING_RAW_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RAW_READER_HPP

#include <iosfwd>
#include "image/image.hpp"

namespace image::encoding::raw {

class Reader {
  public:
    static Image* read(std::istream& stream);

  private:
};

}  // namespace image::encoding::raw

#endif
