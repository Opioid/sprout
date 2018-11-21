#ifndef SU_CORE_RESOURCE_MANAGER_HPP
#define SU_CORE_RESOURCE_MANAGER_HPP

#include <map>
#include <memory>
#include "base/memory/variant_map.hpp"

namespace file {
class System;
}

namespace thread {
class Pool;
}

namespace resource {

class Cache;

template <typename T>
class Typed_cache;

template <typename T>
class Provider;

class Manager {
  public:
    using Variant_map = memory::Variant_map;
    template <typename T>
    using Ptr = std::shared_ptr<T>;

    Manager(file::System& filesystem, thread::Pool& thread_pool) noexcept;

    ~Manager() noexcept;

    file::System& filesystem() noexcept;

    thread::Pool& thread_pool() noexcept;

    template <typename T>
    void register_provider(Provider<T>& provider) noexcept;

    template <typename T>
    Ptr<T> load(std::string const& filename, Variant_map const& options = Variant_map());

    template <typename T>
    Ptr<T> load(std::string const& name, void const* data, std::string const& mount_folder,
                Variant_map const& options = Variant_map());

    template <typename T>
    Ptr<T> get(std::string const& filename, Variant_map const& options = Variant_map()) noexcept;

    template <typename T>
    void store(std::string const& name, Ptr<T> resource,
               Variant_map const& options = Variant_map()) noexcept;

    template <typename T>
    size_t num_bytes() const noexcept;

  private:
    template <typename T>
    const Typed_cache<T>* typed_cache() const noexcept;

    template <typename T>
    Typed_cache<T>* typed_cache() noexcept;

    file::System& filesystem_;

    thread::Pool& thread_pool_;

    std::map<uint32_t, Cache*> caches_;
};

}  // namespace resource

#endif
