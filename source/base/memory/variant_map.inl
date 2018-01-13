#ifndef SU_BASE_MEMORY_VARIANT_MAP_INL
#define SU_BASE_MEMORY_VARIANT_MAP_INL

#include "variant_map.hpp"

namespace memory {

template<typename T>
bool Variant_map::query(std::string_view key, T& value) const {
	const auto i = map_.find(key);

	if (map_.end() == i) {
		return false;
	}

	auto& variant = i->second;

	if (Variant::Type::Uint != variant.type) {
		return false;
	}

	value = static_cast<T>(variant.uint_value);

	return true;
}

inline bool Variant_map::query(std::string_view key, bool& value) const {
	const auto i = map_.find(key);

	if (map_.end() == i) {
		return false;
	}

	auto& variant = i->second;

	if (Variant::Type::Bool != variant.type) {
		return false;
	}

	value = variant.bool_value;

	return true;
}

inline bool Variant_map::query(std::string_view key, int32_t& value) const {
	const auto i = map_.find(key);

	if (map_.end() == i) {
		return false;
	}

	auto& variant = i->second;

	if (Variant::Type::Int != variant.type) {
		return false;
	}

	value = variant.int_value;

	return true;
}

inline bool Variant_map::query(std::string_view key, uint32_t& value) const {
	const auto i = map_.find(key);

	if (map_.end() == i) {
		return false;
	}

	auto& variant = i->second;

	if (Variant::Type::Uint != variant.type) {
		return false;
	}

	value = variant.uint_value;

	return true;
}

inline bool Variant_map::query(std::string_view key, float& value) const {
	const auto i = map_.find(key);

	if (map_.end() == i) {
		return false;
	}

	auto& variant = i->second;

	if (Variant::Type::Float != variant.type) {
		return false;
	}

	value = variant.float_value;

	return true;
}

template<typename T>
void Variant_map::set(const std::string& key, T value) {
	map_[key] = Variant(static_cast<uint32_t>(value));
}

inline void Variant_map::set(const std::string& key, bool value) {
	map_[key] = Variant(value);
}

inline void Variant_map::set(const std::string& key, int32_t value) {
	map_[key] = Variant(value);
}

inline void Variant_map::set(const std::string& key, uint32_t value) {
	map_[key] = Variant(value);
}

inline void Variant_map::set(const std::string& key, float value) {
	map_[key] = Variant(value);
}

inline void Variant_map::inherit(const Variant_map& other, const std::string& key) {
	const auto i = other.map_.find(key);

	if (other.map_.end() == i) {
		return;
	}

	map_[key] = i->second;
}

inline void Variant_map::inherit_except(const Variant_map& other, std::string_view key) {
	for (auto i : other.map_) {
		if (i.first != key) {
			map_.insert(map_.end(), i);
		}
	}
}

}

#endif
