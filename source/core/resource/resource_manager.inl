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
bool Manager::deprecate_frame_dependant() {
    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->deprecate_frame_dependant();
    }

    return false;
}

template <typename T>
void Manager::reload_frame_dependant() {
    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        cache->reload_frame_dependant(*this);
    }
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& filename, Variants const& options) {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->load(filename, options, *this);
    }

    return Resource_ptr<T>::Null();
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& filename, Variants const& options,
                              std::string& resolved_name) {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->load(filename, options, *this, resolved_name);
    }

    return Resource_ptr<T>::Null();
}

template <typename T>
Resource_ptr<T> Manager::load(std::string const& name, void const* data,
                              std::string const& source_name, Variants const& options) {
    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->load(name, data, source_name, options, *this);
    }

    return Resource_ptr<T>::Null();
}

template <typename T>
Resource_ptr<T> Manager::get(std::string const& filename, Variants const& options) {
    if (filename.empty()) {
        return Resource_ptr<T>::Null();
    }

    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        cache->get(filename, options);
    }

    return Resource_ptr<T>::Null();
}

template <typename T>
T* Manager::get(uint32_t id) const {
    if (Typed_cache<T> const* cache = typed_cache<T>(); cache) {
        return cache->get(id);
    }

    return nullptr;
}

template <typename T>
uint32_t Manager::store(T* resource) {
    if (!resource) {
        return resource::Null;
    }

    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->store(resource);
    }

    return resource::Null;
}

template <typename T>
uint32_t Manager::store(std::string const& name, T* resource, Variants const& options) {
    if (name.empty()) {
        return store(resource);
    }

    if (!resource) {
        return resource::Null;
    }

    if (Typed_cache<T>* cache = typed_cache<T>(); cache) {
        return cache->store(name, options, resource);
    }

    return resource::Null;
}

template <typename T>
Typed_cache<T> const* Manager::typed_cache() const {
    if (auto const cache = caches_.find(T::identifier()); caches_.end() != cache) {
        return static_cast<Typed_cache<T> const*>(cache->second);
    }

    return nullptr;
}

template <typename T>
Typed_cache<T>* Manager::typed_cache() {
    if (auto cache = caches_.find(T::identifier()); caches_.end() != cache) {
        return static_cast<Typed_cache<T>*>(cache->second);
    }

    return nullptr;
}

}  // namespace resource

#endif
