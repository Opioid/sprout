#ifndef SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP

#include <iosfwd>

namespace image {

class Image;

namespace encoding::rgbe {

class Reader {
  public:
    static Image* read(std::istream& stream);
};

}  // namespace encoding::rgbe
}  // namespace image

#endif
