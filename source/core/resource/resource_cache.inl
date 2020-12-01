#ifndef SU_CORE_RESOURCE_CACHE_INL
#define SU_CORE_RESOURCE_CACHE_INL

#include "base/memory/variant_map.inl"
#include "core/logging/logging.hpp"
#include "file/file_system.hpp"
#include "resource.hpp"
#include "resource_cache.hpp"
#include "resource_provider.hpp"

#include <string_view>

#ifdef SU_DEBUG
#include <sstream>
#endif

namespace resource {

template <typename T>
Typed_cache<T>::Typed_cache(Provider<T>& provider) : provider_(provider) {}

template <typename T>
Typed_cache<T>::~Typed_cache() {
    for (auto r : resources_) {
        delete r;
    }
}

template <typename T>
std::vector<T*> const& Typed_cache<T>::resources() const {
    return resources_;
}

template <typename T>
uint32_t Typed_cache<T>::load(std::string const& filename, Variants const& options,
                                     Manager& manager) {
    std::string resolved_name;

    return load(filename, options, manager, resolved_name);
}

template <typename T>
uint32_t Typed_cache<T>::load(std::string const& filename, Variants const& options,
                                     Manager& resources, std::string& resolved_name) {
    auto const key = std::make_pair(filename, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        if (check_up_to_date(entry)) {
            return entry.id;
        }
    }

    auto resource = provider_.load(filename, options, resources, resolved_name);
    if (!resource) {
        return resource::Null;
    }

    std::error_code ec;
    auto const      last_write = std::filesystem::last_write_time(resolved_name, ec);

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    entries_.insert_or_assign(key, Entry{id, generation_, resolved_name, last_write});

#ifdef SU_DEBUG
    std::ostringstream stream;
    stream << "Loaded " << T::identifier() << " resource \"" << resolved_name << "\"";
    LOGGING_VERBOSE(stream.str());
#endif

    return id;
}

template <typename T>
uint32_t Typed_cache<T>::load(std::string const& name, void const* data,
                                     std::string const& source_name, Variants const& options,
                                     Manager& resources) {
    auto resource = provider_.load(data, source_name, options, resources);
    if (!resource) {
        return resource::Null;
    }

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    if (!name.empty()) {
        auto const key = std::make_pair(name, options);

        auto const last_write = std::filesystem::last_write_time(source_name);

        entries_.insert_or_assign(key, Entry{id, generation_, source_name, last_write});
    }

    return id;
}

template <typename T>
uint32_t Typed_cache<T>::get(std::string const& filename, Variants const& options) {
    auto const key = std::make_pair(filename, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        if (check_up_to_date(entry)) {
            return entry.id;
        }
    }

    return resource::Null;
}

template <typename T>
T* Typed_cache<T>::get(uint32_t id) const {
    if (id >= uint32_t(resources_.size())) {
        return nullptr;
    }

    return resources_[id];
}

template <typename T>
uint32_t Typed_cache<T>::store(T* resource) {
    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    return id;
}

template <typename T>
uint32_t Typed_cache<T>::store(std::string const& name, Variants const& options, T* resource) {
    auto const key = std::make_pair(name, options);

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    entries_.insert_or_assign(key, Entry{id, generation_, "", std::filesystem::file_time_type()});

    return id;
}

template <typename T>
bool Typed_cache<T>::check_up_to_date(Entry& entry) const {
    if (entry.generation == generation_ || entry.source_name.empty()) {
        return true;
    }

    if (std::filesystem::last_write_time(entry.source_name) == entry.last_write) {
        entry.generation = generation_;
        return true;
    }

    return false;
}

}  // namespace resource

#endif
