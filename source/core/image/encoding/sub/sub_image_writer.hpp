#ifndef SU_CORE_IMAGE_ENCODING_SUB_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_SUB_WRITER_HPP

#include <string>

namespace image {

class Image;

namespace encoding::sub {

class Writer {
  public:
    static void write(std::string const& filename, Image const& image);
};

}  // namespace encoding::sub
}  // namespace image

#endif
