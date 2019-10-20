#ifndef SU_CORE_RESOURCE_MANAGER_INL
#define SU_CORE_RESOURCE_MANAGER_INL

#include "resource.hpp"
#include "resource_cache.inl"
#include "resource_manager.hpp"
#include "resource_provider.inl"

namespace resource {

template <typename T>
std::vector<T*> const& Manager::register_provider(Provider<T>& provider) noexcept {
    std::string const id = T::identifier();

    if (auto old = caches_.find(id); caches_.end() != old) {
        delete old->second;
    }

    auto cache = new Typed_cache<T>(provider);

    caches_[id] = cache;

    return cache->resources();
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& filename, Variant_map const& options) noexcept {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->load(filename, options, *this);
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& filename, Variant_map const& options,
                              std::string& resolved_name) noexcept {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->load(filename, options, *this, resolved_name);
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& name, void const* data,
                              std::string const& source_name, Variant_map const& options) noexcept {
    if (name.empty()) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->load(name, data, source_name, options, *this);
}

template <typename T>
Resource_ptr<T> Manager::get(std::string const& filename, Variant_map const& options) noexcept {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->get(filename, options);
}

template <typename T>
Resource_ptr<T> Manager::store(T* resource) noexcept {
    if (!resource) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->store(resource);
}

template <typename T>
Resource_ptr<T> Manager::store(std::string const& name, T* resource,
                               Variant_map const& options) noexcept {
    if (name.empty() || !resource) {
        return Resource_ptr<T>::Null();
    }

    Typed_cache<T>* cache = typed_cache<T>();

    // a provider for this resource type was never registered
    if (!cache) {
        return Resource_ptr<T>::Null();
    }

    return cache->store(name, options, resource);
}

template <typename T>
size_t Manager::num_bytes() const noexcept {
    Typed_cache<T> const* cache = typed_cache<T>();
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
