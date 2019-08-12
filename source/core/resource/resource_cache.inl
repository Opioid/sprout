#ifndef SU_CORE_RESOURCE_CACHE_INL
#define SU_CORE_RESOURCE_CACHE_INL

#include "base/memory/variant_map.inl"
#include "core/logging/logging.hpp"
#include "file/file_system.hpp"
#include "resource_cache.hpp"

#include <sstream>
#include <string_view>

namespace resource {

template <typename T>
Typed_cache<T>::Typed_cache(Provider<T>& provider) noexcept : provider_(provider) {}

template <typename T>
Typed_cache<T>::~Typed_cache() noexcept {
    for (auto r : resources_) {
        delete r.second.data;
    }
}

template <typename T>
T* Typed_cache<T>::load(std::string const& filename, memory::Variant_map const& options,
                        Manager& manager) noexcept {
    std::string resolved_name;

    return load(filename, options, manager, resolved_name);
}

template <typename T>
T* Typed_cache<T>::load(std::string const& filename, memory::Variant_map const& options,
                        Manager& manager, std::string& resolved_name) noexcept {
    auto const key = std::make_pair(filename, options);

    if (auto cached = resources_.find(key); resources_.end() != cached) {
        auto const& entry = cached->second;

        if (is_up_to_date(entry)) {
            return entry.data;
        }
    }

    auto resource = provider_.load(filename, options, manager, resolved_name);
    if (!resource) {
        return nullptr;
    }

    auto const last_write = std::filesystem::last_write_time(resolved_name);

    resources_.insert_or_assign(key, Entry{resource, resolved_name, generation_, last_write});

    if (logging::is_verbose()) {
        std::stringstream stream;
        stream << "Loaded " << T::identifier() << " resource \"" << filename << "\"";
        logging::verbose(stream.str());
    }

    return resource;
}

template <typename T>
T* Typed_cache<T>::load(std::string const& name, void const* data, std::string const& source_name,
                        memory::Variant_map const& options, Manager& manager) noexcept {
    auto const key = std::make_pair(name, options);

    auto resource = provider_.load(data, source_name, options, manager);
    if (!resource) {
        return nullptr;
    }

    auto const last_write = std::filesystem::last_write_time(source_name);

    resources_.insert_or_assign(key, Entry{resource, source_name, generation_, last_write});

    return resource;
}

template <typename T>
T* Typed_cache<T>::get(std::string const& filename, memory::Variant_map const& options) noexcept {
    auto const key = std::make_pair(filename, options);

    if (auto cached = resources_.find(key); resources_.end() != cached) {
        auto const& entry = cached->second;

        if (is_up_to_date(entry)) {
            return entry.data;
        }

        return nullptr;
    }

    return nullptr;
}

template <typename T>
void Typed_cache<T>::store(std::string const& name, memory::Variant_map const& options,
                           T* resource) noexcept {
    auto const key = std::make_pair(name, options);

    resources_.insert_or_assign(
        key, Entry{resource, "", generation_, std::filesystem::file_time_type()});
}

template <typename T>
size_t Typed_cache<T>::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (auto r : resources_) {
        num_bytes += provider_.num_bytes(r.second.data);
    }

    num_bytes += provider_.num_bytes();

    return num_bytes;
}

template <typename T>
bool Typed_cache<T>::is_up_to_date(Entry const& entry) const noexcept {
    if (entry.generation == generation_ || entry.source_name.empty()) {
        return true;
    }

    if (std::filesystem::last_write_time(entry.source_name) == entry.last_write) {
        return true;
    }

    return false;
}

}  // namespace resource

#endif
