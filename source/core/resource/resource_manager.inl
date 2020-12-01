#ifndef SU_CORE_RESOURCE_MANAGER_INL
#define SU_CORE_RESOURCE_MANAGER_INL

#include "resource.hpp"
#include "resource_cache.inl"
#include "resource_manager.hpp"
#include "resource_provider.inl"

namespace resource {

template <typename T>
std::vector<T*> const& Manager::register_provider(Provider<T>& provider) {
    std::string const id = T::identifier();

    if (auto old = caches_.find(id); caches_.end() != old) {
        delete old->second;
    }

    auto cache = new Typed_cache<T>(provider);

    caches_[id] = cache;

    return cache->resources();
}

template <typename T>
uint32_t Manager::load(std::string const& filename, Variants const& options) {
    if (filename.empty()) {
        return resource::Null;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->load(filename, options, *this);
}

template <typename T>
uint32_t Manager::load(std::string const& filename, Variants const& options,
                              std::string& resolved_name) {
    if (filename.empty()) {
        return resource::Null;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->load(filename, options, *this, resolved_name);
}

template <typename T>
uint32_t Manager::load(std::string const& name, void const* data,
                              std::string const& source_name, Variants const& options) {
    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->load(name, data, source_name, options, *this);
}

template <typename T>
uint32_t Manager::get(std::string const& filename, Variants const& options) {
    if (filename.empty()) {
        return resource::Null;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->get(filename, options);
}

template <typename T>
T* Manager::get(uint32_t id) const {
    Typed_cache<T> const* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return nullptr;
    }

    return cache->get(id);
}

template <typename T>
uint32_t Manager::store(T* resource) {
    if (!resource) {
        return resource::Null;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->store(resource);
}

template <typename T>
uint32_t Manager::store(std::string const& name, T* resource, Variants const& options) {
    if (name.empty() || !resource) {
        return resource::Null;
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return resource::Null;
    }

    return cache->store(name, options, resource);
}

template <typename T>
Typed_cache<T> const* Manager::typed_cache() const {
    auto const cache = caches_.find(T::identifier());

    if (caches_.end() == cache) {
        return nullptr;
    }

    return static_cast<Typed_cache<T> const*>(cache->second);
}

template <typename T>
Typed_cache<T>* Manager::typed_cache() {
    auto cache = caches_.find(T::identifier());

    if (caches_.end() == cache) {
        return nullptr;
    }

    return static_cast<Typed_cache<T>*>(cache->second);
}

}  // namespace resource

#endif
