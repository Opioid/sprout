#ifndef SU_CORE_EXPORTING_SINK_HPP
#define SU_CORE_EXPORTING_SINK_HPP

#include "image/typed_image_fwd.hpp"
#include "rendering/sensor/aov/property.hpp"

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace exporting {

class Sink {
  public:
    using AOV = rendering::sensor::aov::Property;

    virtual ~Sink() = default;

    virtual void write(image::Float4 const& image, AOV aov, uint32_t frame, Threads& threads) = 0;
};

}  // namespace exporting

#endif
