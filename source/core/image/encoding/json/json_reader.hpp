#ifndef SU_CORE_IMAGE_ENCODING_JSON_READER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_READER_HPP

#include <iosfwd>
#include <memory>
#include <string>
#include "image/typed_image.hpp"

namespace image::encoding::json {

class Reader {
  public:
    std::shared_ptr<Image> read(std::istream& stream, std::string& error) const;

  private:
};

}  // namespace image::encoding::json

#endif
