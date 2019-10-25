#ifndef CORE_IMAGE_WRITER_HPP
#define CORE_IMAGE_WRITER_HPP

#include <iosfwd>
#include <string>
#include "typed_image_fwd.hpp"

namespace thread {
class Pool;
}

namespace image {

class Writer {
  public:
    virtual ~Writer();

    virtual std::string file_extension() const = 0;

    virtual bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) = 0;
};

}  // namespace image

#endif
