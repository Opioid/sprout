#include "variant_map.hpp"

namespace memory {

bool Variant_map::operator<(Variant_map const& other) const noexcept {
    return map_ < other.map_;
}

Variant_map::Variant::Variant(bool bool_value) noexcept
    : type(Type::Bool), bool_value(bool_value) {}
Variant_map::Variant::Variant(int32_t int_value) noexcept : type(Type::Int), int_value(int_value) {}
Variant_map::Variant::Variant(uint32_t uint_value) noexcept
    : type(Type::Uint), uint_value(uint_value) {}
Variant_map::Variant::Variant(float float_value) noexcept
    : type(Type::Float), float_value(float_value) {}

bool Variant_map::Variant::operator<(Variant const& other) const noexcept {
    if (type != other.type) {
        return type < other.type;
    }

    return uint_value < other.uint_value;
}

}  // namespace memory
