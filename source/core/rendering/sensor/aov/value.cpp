#include "value.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor::aov {

Value::Value() : num_slots_(0), slots_(nullptr) {}

Value::~Value() {
    delete[] slots_;
}

void Value::init(Mapping mapping, uint32_t num_slots) {
    mapping_ = mapping;

    num_slots_ = uint8_t(num_slots);

    slots_ = new Slot[num_slots];

    for (uint32_t i = 0; i < Max_slots; ++i) {
        uint8_t const slot = mapping.m[i];

        if (slot != 255) {
            slots_[slot].operation = aov::operation(Property(i));
        }
    }
}

void Value::clear() {
    for (uint32_t i = 0, len = uint32_t(num_slots_); i < len; ++i) {
        float const iv = initial_value(slots_[i].operation);

        slots_[i].v = float4(iv);
    }
}

Value_pool::Value_pool() : num_slots_(0), properties_(nullptr), values_(nullptr) {}

Value_pool::~Value_pool() {
    delete[] properties_;
    delete[] values_;
}

void Value_pool::configure(uint32_t num_slots, Property const* properties) {
    if (0 == num_slots) {
        return;
    }

    num_slots_ = uint8_t(num_slots);

    properties_ = new Property[num_slots];

    for (uint32_t i = 0; i < num_slots; ++i) {
        properties_[i] = properties[i];
    }

    for (uint32_t i = 0; i < Value::Max_slots; ++i) {
        mapping_.m[i] = 255;
    }

    for (uint32_t i = 0; i < num_slots; ++i) {
        auto const p = properties[i];

        mapping_.m[uint32_t(p)] = uint8_t(i);
    }
}

void Value_pool::init(uint32_t num_values) {
    if (0 == num_slots_) {
        return;
    }

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
    return 0 == num_slots_ ? nullptr : &values_[id];
}

}  // namespace rendering::sensor::aov
