#ifndef SU_CORE_IMAGE_ENCODING_EXR_READER_HPP
#define SU_CORE_IMAGE_ENCODING_EXR_READER_HPP

#include <iosfwd>

namespace image {

class Image;

namespace encoding::exr {

class Reader {
  public:
    static Image* read(std::istream& stream);
};

}  // namespace encoding::exr
}  // namespace image

#endif
