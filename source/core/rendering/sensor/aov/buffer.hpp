#ifndef SU_CORE_RENDERING_SENSOR_AOV_BUFFER_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_BUFFER_HPP

#include "base/math/vector4.hpp"
#include "value.hpp"

namespace rendering::sensor::aov {

class Buffer {
  public:
    Buffer();

    ~Buffer();

    void resize(int2 dimensions, Value_pool const& aovs);

    void clear();

    void add_pixel(int32_t id, uint32_t slot, float3 const& value, float weight);

    void add_pixel_atomic(int32_t id, uint32_t slot, float3 const& value, float weight);

    float3 value(int32_t id, uint32_t slot) const;

  private:
    uint32_t buffers_len_;
    uint32_t buffer_len_;

    float3** buffers_;
};

}  // namespace rendering::sensor::aov

#endif
