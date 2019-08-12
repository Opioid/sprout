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

        if (entry.generation == generation_ || entry.resolved_name.empty()) {
            return entry.data;
        }

        auto const last_write = std::experimental::filesystem::last_write_time(entry.resolved_name);
        if (last_write == entry.last_write) {
            return entry.data;
        }
    }

    auto resource = provider_.load(filename, options, manager, resolved_name);
    if (!resource) {
        return nullptr;
    }

    auto const last_write = std::experimental::filesystem::last_write_time(resolved_name);

    resources_.insert_or_assign(key, Entry{resource, resolved_name, generation_, last_write});

    if (logging::is_verbose()) {
        std::stringstream stream;
        stream << "Loaded " << T::identifier() << " resource \"" << filename << "\"";
        logging::verbose(stream.str());
    }

    return resource;
}

template <typename T>
T* Typed_cache<T>::load(std::string const& name, void const* data, std::string_view mount_folder,
                        memory::Variant_map const& options, Manager& manager) noexcept {
    auto const key = std::make_pair(name, options);

    auto resource = provider_.load(data, mount_folder, options, manager);
    if (!resource) {
        return nullptr;
    }

    resources_.insert_or_assign(
        key, Entry{resource, "", generation_, std::experimental::filesystem::file_time_type()});

    return resource;
}

template <typename T>
T* Typed_cache<T>::get(std::string const& filename, memory::Variant_map const& options) noexcept {
    auto const key = std::make_pair(filename, options);

    if (auto cached = resources_.find(key); resources_.end() != cached) {
        auto const& entry = cached->second;

        if (entry.generation < generation_ && !entry.resolved_name.empty()) {
            auto const last_write = std::experimental::filesystem::last_write_time(
                entry.resolved_name);

            if (last_write > entry.last_write) {
                // the entry is outdated
                return nullptr;
            }
        }

        return entry.data;
    }

    return nullptr;
}

template <typename T>
void Typed_cache<T>::store(std::string const& name, memory::Variant_map const& options,
                           T* resource) noexcept {
    auto const key = std::make_pair(name, options);

    resources_.insert_or_assign(
        key, Entry{resource, "", generation_, std::experimental::filesystem::file_time_type()});
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

}  // namespace resource

#endif
