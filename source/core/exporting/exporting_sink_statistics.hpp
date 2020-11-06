#ifndef SU_CORE_EXPORTING_SINK_STATISTICS_HPP
#define SU_CORE_EXPORTING_SINK_STATISTICS_HPP

#include "exporting_sink.hpp"

namespace exporting {

class Statistics final : public Sink {
  public:
    Statistics();

    ~Statistics() final;

    void write(image::Float4 const& image, AOV aov, uint32_t frame, Threads& threads) final;
};

}  // namespace exporting

#endif
