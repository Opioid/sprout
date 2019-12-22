#ifndef SU_CORE_RESOURCE_CACHE_INL
#define SU_CORE_RESOURCE_CACHE_INL

#include "base/memory/variant_map.inl"
#include "core/logging/logging.hpp"
#include "file/file_system.hpp"
#include "resource.hpp"
#include "resource_cache.hpp"
#include "resource_provider.hpp"

#include <sstream>
#include <string_view>

namespace resource {

template <typename T>
Typed_cache<T>::Typed_cache(Provider<T>& provider) noexcept : provider_(provider) {}

template <typename T>
Typed_cache<T>::~Typed_cache() noexcept {
    for (auto r : resources_) {
        delete r;
    }
}

template <typename T>
std::vector<T*> const& Typed_cache<T>::resources() const noexcept {
    return resources_;
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const&         filename,
                                     memory::Variant_map const& options,
                                     Manager&                   manager) noexcept {
    std::string resolved_name;

    return load(filename, options, manager, resolved_name);
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const&         filename,
                                     memory::Variant_map const& options, Manager& manager,
                                     std::string& resolved_name) noexcept {
    auto const key = std::make_pair(filename, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        if (check_up_to_date(entry)) {
            uint32_t const id = entry.id;
            return {resources_[id], id};
        }
    }

    auto resource = provider_.load(filename, options, manager, resolved_name);
    if (!resource) {
        return Resource_ptr<T>::Null();
    }

    std::error_code ec;
    auto const      last_write = std::filesystem::last_write_time(resolved_name, ec);

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    entries_.insert_or_assign(key, Entry{id, generation_, resolved_name, last_write});

    if (logging::is_verbose()) {
        std::stringstream stream;
        stream << "Loaded " << T::identifier() << " resource \"" << filename << "\"";
        logging::verbose(stream.str());
    }

    return {resource, id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::load(std::string const& name, void const* data,
                                     std::string const&         source_name,
                                     memory::Variant_map const& options,
                                     Manager&                   manager) noexcept {
    auto resource = provider_.load(data, source_name, options, manager);
    if (!resource) {
        return Resource_ptr<T>::Null();
    }

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    if (!name.empty()) {
        auto const key = std::make_pair(name, options);

        auto const last_write = std::filesystem::last_write_time(source_name);

        entries_.insert_or_assign(key, Entry{id, generation_, source_name, last_write});
    }

    return {resource, id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::get(std::string const&         filename,
                                    memory::Variant_map const& options) noexcept {
    auto const key = std::make_pair(filename, options);

    if (auto cached = entries_.find(key); entries_.end() != cached) {
        auto& entry = cached->second;

        if (check_up_to_date(entry)) {
            uint32_t const id = entry.id;
            return {resources_[id], id};
        }

        return Resource_ptr<T>::Null();
    }

    return Resource_ptr<T>::Null();
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::get(uint32_t id) const noexcept {
    if (id >= uint32_t(resources_.size())) {
        return Resource_ptr<T>::Null();
    }

    return {resources_[id], id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::store(T* resource) noexcept {
    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    return {resource, id};
}

template <typename T>
Resource_ptr<T> Typed_cache<T>::store(std::string const& name, memory::Variant_map const& options,
                                      T* resource) noexcept {
    auto const key = std::make_pair(name, options);

    resources_.push_back(resource);

    uint32_t const id = uint32_t(resources_.size()) - 1;

    entries_.insert_or_assign(key, Entry{id, generation_, "", std::filesystem::file_time_type()});

    return {resource, id};
}

template <typename T>
size_t Typed_cache<T>::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (auto r : resources_) {
        num_bytes += provider_.num_bytes(r);
    }

    num_bytes += provider_.num_bytes();

    return num_bytes;
}

template <typename T>
bool Typed_cache<T>::check_up_to_date(Entry& entry) const noexcept {
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
