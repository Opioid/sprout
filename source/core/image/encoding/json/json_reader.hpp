#ifndef SU_CORE_IMAGE_ENCODING_JSON_READER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_READER_HPP

#include <iosfwd>
#include <string>
#include "image/image.hpp"

namespace image::encoding::json {

class Reader {
  public:
    static Image* read(std::istream& stream, std::string const& filename) noexcept;

  private:
};

}  // namespace image::encoding::json

#endif
