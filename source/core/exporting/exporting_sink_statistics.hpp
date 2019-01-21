#ifndef SU_CORE_EXPORTING_SINK_STATISTICS_HPP
#define SU_CORE_EXPORTING_SINK_STATISTICS_HPP

#include "exporting_sink.hpp"

namespace image {
class Writer;
}

namespace exporting {

class Statistics : public Sink {
  public:
    Statistics() noexcept;

    ~Statistics() noexcept override final;

    void write(image::Float4 const& image, uint32_t frame,
               thread::Pool& pool) noexcept override final;
};

}  // namespace exporting

#endif
