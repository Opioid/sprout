#ifndef SU_CORE_IMAGE_ENCODING_RAW_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RAW_READER_HPP

#include <iosfwd>
#include <memory>
#include "image/image.hpp"

namespace image::encoding::raw {

class Reader {
  public:
    std::shared_ptr<Image> read(std::istream& stream) const;

  private:
};

}  // namespace image::encoding::raw

#endif
