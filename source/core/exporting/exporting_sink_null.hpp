#ifndef SU_CORE_EXPORTING_SINK_NULL_HPP
#define SU_CORE_EXPORTING_SINK_NULL_HPP

#include "exporting_sink.hpp"

namespace exporting {

class Null : public Sink {
  public:
    Null() noexcept;

    ~Null() noexcept final;

    void write(image::Float4 const& image, uint32_t frame, thread::Pool& threads) noexcept final;
};

}  // namespace exporting

#endif
