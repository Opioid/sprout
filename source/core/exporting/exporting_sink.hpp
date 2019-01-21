#ifndef SU_CORE_EXPORTING_SINK_HPP
#define SU_CORE_EXPORTING_SINK_HPP

#include "image/typed_image_fwd.hpp"

namespace thread {
class Pool;
}

namespace exporting {

class Sink {
  public:
    virtual ~Sink() noexcept {}

    virtual void write(image::Float4 const& image, uint32_t frame, thread::Pool& pool) noexcept = 0;
};

}  // namespace exporting

#endif
