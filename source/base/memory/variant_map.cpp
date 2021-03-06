#include "variant_map.hpp"

namespace memory {

bool Variant_map::operator<(Variant_map const& other) const {
    return map_ < other.map_;
}

Variant_map::Variant::Variant(bool bool_value) : type(Type::Bool), bool_value(bool_value) {}
Variant_map::Variant::Variant(int32_t int_value) : type(Type::Int), int_value(int_value) {}
Variant_map::Variant::Variant(uint32_t uint_value) : type(Type::Uint), uint_value(uint_value) {}
Variant_map::Variant::Variant(float float_value) : type(Type::Float), float_value(float_value) {}

Variant_map Variant_map::clone_except(std::string_view key) const {
    Variant_map other;

    for (auto i : map_) {
        if (i.first != key) {
            other.map_.insert(other.map_.end(), i);
        }
    }

    return other;
}

bool Variant_map::Variant::operator<(Variant const& other) const {
    if (type != other.type) {
        return type < other.type;
    }

    return uint_value < other.uint_value;
}

}  // namespace memory
