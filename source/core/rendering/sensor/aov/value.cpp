#include "value.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor::aov {

Value::Value() : num_slots_(0), slots_(nullptr) {}

Value::~Value() {
    delete[] slots_;
}

void Value::init(Mapping mapping, uint32_t num_slots) {
    mapping_ = mapping;

    num_slots_ = num_slots;

    slots_ = new Slot[num_slots];
}

void Value::clear() {
    for (uint32_t i = 0, len = num_slots_; i < len; ++i) {
        slots_[i].v[0] = 0.f;
        slots_[i].v[1] = 0.f;
        slots_[i].v[2] = 0.f;
    }
}

Value_pool::Value_pool() : values_(nullptr) {}

Value_pool::~Value_pool() {
    delete[] properties_;
    delete[] values_;
}

void Value_pool::configure(uint32_t num_slots, Property const* properties) {
    num_slots_ = num_slots;

    properties_ = new Property[num_slots];

    for (uint32_t i = 0; i < num_slots; ++i) {
        properties_[i] = properties[i];
    }

    for (uint32_t i = 0; i < 4; ++i) {
        mapping_.m[i] = 255;
    }

    for (uint32_t i = 0; i < num_slots; ++i) {
        auto const p = properties[i];

        mapping_.m[uint32_t(p)] = i;
    }
}

void Value_pool::init(uint32_t num_values) {
    values_ = new Value[num_values];

    for (uint32_t i = 0; i < num_values; ++i) {
        values_[i].init(mapping_, num_slots_);
    }
}

Value::Mapping const& Value_pool::mapping() const {
    return mapping_;
}

uint32_t Value_pool::num_slots() const {
    return num_slots_;
}

Property Value_pool::property(uint32_t slot) const {
    return properties_[slot];
}

Value* Value_pool::get(uint32_t id) const {
    return &values_[id];
}

}  // namespace rendering::sensor::aov
