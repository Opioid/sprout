#ifndef SU_CORE_RENDERING_SENSOR_AOV_VALUE_INL
#define SU_CORE_RENDERING_SENSOR_AOV_VALUE_INL

#include "base/math/vector4.inl"
#include "value.hpp"

namespace rendering::sensor::aov {

inline uint32_t Value::num_slots() const {
    return num_slots_;
}

inline bool Value::active(Property aov) const {
    uint8_t const id = mapping_.m[uint32_t(aov)];

    return id != 255;
}

inline void Value::insert(float v, Property aov) {
    uint32_t const id = mapping_.m[uint32_t(aov)];

    if (255 == id) {
        return;
    }

    Slot& slot = slots_[id];

    slot.v[0] = v;
}

inline void Value::insert(float3_p v, Property aov) {
    insert(float4(v, 0.f), aov);
}

inline void Value::insert(float4_p v, Property aov) {
    uint32_t const id = mapping_.m[uint32_t(aov)];

    if (255 == id) {
        return;
    }

    Slot& slot = slots_[id];

    slot.v = v;
}

inline float4 Value::value(uint32_t id) const {
    return slots_[id].v;
}

inline Operation Value::operation(uint32_t id) const {
    return slots_[id].operation;
}

inline float Value::param(Property aov) const {
    uint32_t const id = mapping_.m[uint32_t(aov)];

    if (255 == id) {
        return 0.f;
    }

    return slots_[id].fparam;
}

}  // namespace rendering::sensor::aov

#endif
