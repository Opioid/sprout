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
void Typed_cache<T>::reload_frame_dependant(Manager& resources) {
    for (auto const& kv : entries_) {
        if (std::string const& filename = kv.first.first; file::System::frame_dependant_name(filename)) {
            std::string resolved_name;
            auto resource = provider_.load(filename, kv.first.second, resources, resolved_name);

            if (resource) {
                uint32_t const id = kv.second.id;

                delete resources_[id];
                resources_[id] = resource;
            }
        }
    }
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const& filename, Variants const& options,
                                     Manager& resources) {
    std::string resolved_name;

    return load(filename, options, resources, resolved_name);
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const& filename, Variants const& options,
                                     Manager& resources, std::string& resolved_name) {
    auto const key = std::make_pair(filename, options);

    uint32_t id = resource::Null;

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        id = entry.id;

        if (check_up_to_date(entry)) {
            return {resources_[id], id};
        }

        delete resources_[id];
        resources_[id] = nullptr;
    }

    auto resource = provider_.load(filename, options, resources, resolved_name);
    if (!resource) {
        return Resource_ptr<T>::Null();
    }

    std::error_code ec;
    auto const      last_write = std::filesystem::last_write_time(resolved_name, ec);

    if (id != resource::Null) {
        resources_[id] = resource;
    } else {
        resources_.push_back(resource);

        id = uint32_t(resources_.size()) - 1;
    }

    entries_.insert_or_assign(key, Entry{id, generation_, resolved_name, last_write});

#ifdef SU_DEBUG
    std::ostringstream stream;
    stream << "Loaded " << T::identifier() << " resource \"" << resolved_name << "\"";
    LOGGING_VERBOSE(stream.str());
#endif

    return {resource, id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const& name, void const* const data,
                                     std::string const& source_name, Variants const& options,
                                     Manager& resources) {
    auto resource = provider_.load(data, source_name, options, resources);
    if (!resource) {
        return Resource_ptr<T>::Null();
    }

    uint32_t id = resource::Null;

    auto const key = std::make_pair(name, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        id = entry.id;
    }

    if (resource::Null == id) {
        resources_.push_back(resource);

        id = uint32_t(resources_.size()) - 1;
    } else {
        delete resources_[id];

        resources_[id] = resource;
    }

    if (!name.empty()) {
        std::error_code ec;
        auto const      last_write = std::filesystem::last_write_time(source_name, ec);

        entries_.insert_or_assign(key, Entry{id, generation_, source_name, last_write});
    }

    return {resource, id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::get(std::string const& name, Variants const& options) {
    auto const key = std::make_pair(name, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        uint32_t const id = entry.id;

        if (check_up_to_date(entry)) {
            return {resources_[id], id};
        }

        return Resource_ptr<T>::Null();
    }

    return Resource_ptr<T>::Null();
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

}  // namespace resource

#endif
