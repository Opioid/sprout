#ifndef SU_CORE_RESOURCE_MANAGER_INL
#define SU_CORE_RESOURCE_MANAGER_INL

#include "resource_cache.inl"
#include "resource_manager.hpp"
#include "resource_provider.inl"

namespace resource {

template <typename T>
void Manager::register_provider(Provider<T>& provider) noexcept {
    std::string const id = T::identifier();

    auto old = caches_.find(id);

    if (caches_.end() != old) {
        delete old->second;
    }

    caches_[id] = new Typed_cache<T>(provider);
}

template <typename T>
T* Manager::load(std::string const& filename, Variant_map const& options) noexcept {
    if (filename.empty()) {
        return nullptr;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return nullptr;
    }

    return cache->load(filename, options, *this);
}

template <typename T>
T* Manager::load(std::string const& filename, Variant_map const& options,
                 std::string& resolved_name) noexcept {
    if (filename.empty()) {
        return nullptr;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return nullptr;
    }

    return cache->load(filename, options, *this, resolved_name);
}

template <typename T>
T* Manager::load(std::string const& name, void const* data, std::string_view mount_folder,
                 std::string source_name, Variant_map const& options) noexcept {
    if (name.empty()) {
        return nullptr;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return nullptr;
    }

    return cache->load(name, data, mount_folder, source_name, options, *this);
}

template <typename T>
T* Manager::get(std::string const& filename, Variant_map const& options) noexcept {
    if (filename.empty()) {
        return nullptr;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return nullptr;
    }

    return cache->get(filename, options);
}

template <typename T>
void Manager::store(std::string const& name, T* resource, Variant_map const& options) noexcept {
    if (name.empty() || !resource) {
        return;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return;
    }

    return cache->store(name, options, resource);
}

template <typename T>
size_t Manager::num_bytes() const noexcept {
    const Typed_cache<T>* cache = typed_cache<T>();
    if (!cache) {
        return 0;
    }

    return cache->num_bytes();
}

template <typename T>
Typed_cache<T> const* Manager::typed_cache() const noexcept {
    auto const cache = caches_.find(T::identifier());

    if (caches_.end() == cache) {
        return nullptr;
    }

    return static_cast<Typed_cache<T> const*>(cache->second);
}

template <typename T>
Typed_cache<T>* Manager::typed_cache() noexcept {
    auto cache = caches_.find(T::identifier());

    if (caches_.end() == cache) {
        return nullptr;
    }

    return static_cast<Typed_cache<T>*>(cache->second);
}

}  // namespace resource

#endif
