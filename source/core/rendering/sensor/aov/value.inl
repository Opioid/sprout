#ifndef SU_CORE_RENDERING_SENSOR_AOV_VALUE_INL
#define SU_CORE_RENDERING_SENSOR_AOV_VALUE_INL

#include "base/math/vector4.inl"
#include "value.hpp"

namespace rendering::sensor::aov {

inline bool Value::empty() const {
    return 0 == num_slots_;
}

inline uint32_t Value::num_slots() const {
    return num_slots_;
}

inline void Value::insert(float3 const& v, Property aov) {
    uint32_t const id = mapping_.m[uint32_t(aov)];

    if (255 == id) {
        return;
    }

    slots_[id].v[0] = v[0];
    slots_[id].v[1] = v[1];
    slots_[id].v[2] = v[2];
}

inline float3 Value::value(uint32_t id) const {
    Slot const& slot = slots_[id];

    return float3(slot.v);
}

}  // namespace rendering::sensor::aov

#endif
