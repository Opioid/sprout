#ifndef CORE_IMAGE_WRITER_HPP
#define CORE_IMAGE_WRITER_HPP

#include "typed_image_fwd.hpp"

#include <iosfwd>
#include <string>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace image {

class Writer {
  public:
    virtual ~Writer();

    virtual std::string file_extension() const = 0;

    virtual bool write(std::ostream& stream, Float4 const& image, Threads& threads) = 0;

    virtual bool write(std::ostream& stream, Float4 const& image, bool data, Threads& threads);
};

}  // namespace image

#endif
