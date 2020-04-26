#ifndef SU_CORE_IMAGE_ENCODING_SUB_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_SUB_WRITER_HPP

#include <iosfwd>

namespace image {

class Image;

namespace encoding::sub {

class Reader {
  public:
    static Image* read(std::istream& stream);
};

}  // namespace encoding::sub
}  // namespace image

#endif
