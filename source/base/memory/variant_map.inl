#ifndef SU_BASE_MEMORY_VARIANT_MAP_INL
#define SU_BASE_MEMORY_VARIANT_MAP_INL

#include "variant_map.hpp"

namespace memory {

template <typename T>
T Variant_map::query(std::string_view key, T def) const {
    auto const i = map_.find(key);

    if (map_.end() == i) {
        return def;
    }

    auto& variant = i->second;

    if (Variant::Type::Uint != variant.type) {
        return def;
    }

    return static_cast<T>(variant.uint_value);
}

inline bool Variant_map::query(std::string_view key, bool def) const {
    auto const i = map_.find(key);

    if (map_.end() == i) {
        return def;
    }

    auto& variant = i->second;

    if (Variant::Type::Bool != variant.type) {
        return def;
    }

    return variant.bool_value;
}

inline int32_t Variant_map::query(std::string_view key, int32_t def) const {
    auto const i = map_.find(key);

    if (map_.end() == i) {
        return def;
    }

    auto& variant = i->second;

    if (Variant::Type::Int != variant.type) {
        return def;
    }

    return variant.int_value;
}

inline uint32_t Variant_map::query(std::string_view key, uint32_t def) const {
    auto const i = map_.find(key);

    if (map_.end() == i) {
        return def;
    }

    auto& variant = i->second;

    if (Variant::Type::Uint != variant.type) {
        return def;
    }

    return variant.uint_value;
}

inline float Variant_map::query(std::string_view key, float def) const {
    auto const i = map_.find(key);

    if (map_.end() == i) {
        return def;
    }

    auto& variant = i->second;

    if (Variant::Type::Float != variant.type) {
        return def;
    }

    return variant.float_value;
}

template <typename T>
void Variant_map::set(std::string const& key, T value) {
    map_.insert_or_assign(key, uint32_t(value));
}

inline void Variant_map::set(std::string const& key, bool value) {
    map_.insert_or_assign(key, value);
}

inline void Variant_map::set(std::string const& key, int32_t value) {
    map_.insert_or_assign(key, value);
}

inline void Variant_map::set(std::string const& key, uint32_t value) {
    map_.insert_or_assign(key, value);
}

inline void Variant_map::set(std::string const& key, float value) {
    map_.insert_or_assign(key, value);
}

}  // namespace memory

#endif
