#ifndef SU_BASE_MEMORY_VARIANT_MAP_INL
#define SU_BASE_MEMORY_VARIANT_MAP_INL

#include "variant_map.hpp"

namespace memory {

template <typename T>
bool Variant_map::query(std::string_view key, T& value) const noexcept {
    auto const i = map_.find(key);

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

inline bool Variant_map::query(std::string_view key, bool& value) const noexcept {
    auto const i = map_.find(key);

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

inline bool Variant_map::query(std::string_view key, int32_t& value) const noexcept {
    auto const i = map_.find(key);

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

inline bool Variant_map::query(std::string_view key, uint32_t& value) const noexcept {
    auto const i = map_.find(key);

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

inline bool Variant_map::query(std::string_view key, float& value) const noexcept {
    auto const i = map_.find(key);

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

template <typename T>
void Variant_map::set(std::string const& key, T value) noexcept {
    map_.emplace(key, static_cast<uint32_t>(value));
}

inline void Variant_map::set(std::string const& key, bool value) noexcept {
    map_.emplace(key, value);
}

inline void Variant_map::set(std::string const& key, int32_t value) noexcept {
    map_.emplace(key, value);
}

inline void Variant_map::set(std::string const& key, uint32_t value) noexcept {
    map_.emplace(key, value);
}

inline void Variant_map::set(std::string const& key, float value) noexcept {
    map_.emplace(key, value);
}

inline void Variant_map::inherit(Variant_map const& other, std::string const& key) noexcept {
    auto const i = other.map_.find(key);

    if (other.map_.end() == i) {
        return;
    }

    map_.insert_or_assign(key, i->second);
}

inline void Variant_map::inherit_except(Variant_map const& other, std::string_view key) noexcept {
    for (auto i : other.map_) {
        if (i.first != key) {
            map_.insert(map_.end(), i);
        }
    }
}

}  // namespace memory

#endif
