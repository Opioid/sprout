#ifndef SU_CORE_RESOURCE_CACHE_INL
#define SU_CORE_RESOURCE_CACHE_INL

#include <sstream>
#include <string_view>
#include "base/memory/variant_map.inl"
#include "core/logging/logging.hpp"
#include "file/file_system.hpp"
#include "resource_cache.hpp"

namespace resource {

inline Cache::~Cache() {}

template <typename T>
Typed_cache<T>::Typed_cache(Provider<T>& provider) : provider_(provider) {}

template <typename T>
Typed_cache<T>::~Typed_cache() {}

template <typename T>
std::shared_ptr<T> Typed_cache<T>::load(std::string const&         filename,
                                        memory::Variant_map const& options, Manager& manager) {
    auto key = std::make_pair(filename, options);

    auto cached = resources_.find(key);
    if (resources_.end() != cached) {
        return cached->second;
    }

    auto resource = provider_.load(filename, options, manager);
    if (!resource) {
        return nullptr;
    }

    resources_[key] = resource;

    if (logging::is_verbose()) {
        std::stringstream stream;
        stream << "Loaded " << provider_.name() << " resource \"" << filename << "\"";
        logging::verbose(stream.str());
    }

    return resource;
}

template <typename T>
std::shared_ptr<T> Typed_cache<T>::load(std::string const& name, void const* data,
                                        std::string_view           mount_folder,
                                        memory::Variant_map const& options, Manager& manager) {
    auto key = std::make_pair(name, options);

    auto resource = provider_.load(data, mount_folder, options, manager);
    if (!resource) {
        return nullptr;
    }

    resources_[key] = resource;

    return resource;
}

template <typename T>
std::shared_ptr<T> Typed_cache<T>::get(std::string const&         filename,
                                       memory::Variant_map const& options) {
    auto key = std::make_pair(filename, options);

    auto cached = resources_.find(key);
    if (resources_.end() != cached) {
        return cached->second;
    }

    return nullptr;
}

template <typename T>
void Typed_cache<T>::store(std::string const& name, memory::Variant_map const& options,
                           std::shared_ptr<T> resource) {
    auto key = std::make_pair(name, options);

    resources_[key] = resource;
}

template <typename T>
size_t Typed_cache<T>::num_bytes() const {
    size_t num_bytes = 0;

    for (auto r : resources_) {
        num_bytes += r.second->num_bytes();
    }

    num_bytes += provider_.num_bytes();

    return num_bytes;
}

}  // namespace resource

#endif
