#ifndef SU_CORE_RENDERING_SENSOR_AOV_BUFFER_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_BUFFER_HPP

#include "property.hpp"
#include "base/math/vector4.hpp"

namespace rendering::sensor::aov {

class Buffer {
public:

    Buffer();

    ~Buffer();

    void resize(int2 dimensions);

    void clear();

    void add_pixel(int32_t id, float4 const& value, float weight, Property aov);

private:

    uint32_t len_;

    float3* shading_normal_;
};

}

#endif
