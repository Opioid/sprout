#ifndef SU_CORE_RENDERING_SENSOR_AOV_VALUE_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_VALUE_HPP

#include "base/math/vector3.hpp"
#include "base/math/vector4.hpp"

namespace rendering::sensor::aov {

class Value {
public:

    void insert(float3 const& v);

    float4 value() const;

private:

    float3 v_;
};

}

#endif
