#include "variant_map.hpp"

namespace memory {

bool Variant_map::operator<(const Variant_map& other) const {
	return map_ < other.map_;
}

Variant_map::Variant::Variant() : type(Type::Unknown) {}
Variant_map::Variant::Variant(bool bool_value) : type(Type::Bool), bool_value(bool_value) {}
Variant_map::Variant::Variant(int32_t int_value) : type(Type::Int), int_value(int_value) {}
Variant_map::Variant::Variant(uint32_t uint_value) : type(Type::Uint), uint_value(uint_value) {}
Variant_map::Variant::Variant(float float_value) : type(Type::Float), float_value(float_value) {}

bool Variant_map::Variant::operator<(const Variant& other) const {
	if (type != other.type) {
		return type < other.type;
	}

	return uint_value < other.uint_value;
}

}
