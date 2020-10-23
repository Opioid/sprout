#include "value.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor::aov {

void Value::insert(float3 const& v) {
    v_ = v;
}

float4 Value::value() const {
    return float4(v_, 1.f);
}

}
