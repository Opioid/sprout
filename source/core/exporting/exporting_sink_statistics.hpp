#ifndef SU_CORE_EXPORTING_SINK_STATISTICS_HPP
#define SU_CORE_EXPORTING_SINK_STATISTICS_HPP

#include "exporting_sink.hpp"

namespace image {
class Writer;
}

namespace exporting {

class Statistics final : public Sink {
  public:
    Statistics();

    ~Statistics() final;

    void write(image::Float4 const& image, uint32_t frame, thread::Pool& threads) final;
};

}  // namespace exporting

#endif
