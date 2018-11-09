#ifndef SU_CORE_IMAGE_ENCODING_JSON_READER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_READER_HPP

#include <iosfwd>
#include <memory>
#include "image/image.hpp"

namespace image::encoding::json {

class Reader {
  public:
    std::shared_ptr<Image> read(std::istream& stream) const;

  private:
};

}  // namespace image::encoding::json

#endif
