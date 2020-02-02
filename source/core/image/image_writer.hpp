#ifndef CORE_IMAGE_WRITER_HPP
#define CORE_IMAGE_WRITER_HPP

#include "typed_image_fwd.hpp"

#include <iosfwd>
#include <string>

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
