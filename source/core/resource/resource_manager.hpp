#ifndef SU_CORE_RESOURCE_MANAGER_HPP
#define SU_CORE_RESOURCE_MANAGER_HPP

#include "base/memory/variant_map.hpp"
#include "file/file_system.hpp"

#include <map>
#include <string>
#include <vector>

namespace thread {
class Pool;
}

namespace resource {

class Cache;

template <typename T>
class Typed_cache;

template <typename T>
struct Resource_ptr;

template <typename T>
class Provider;

class Manager {
  public:
    using Variants = memory::Variant_map;

    Manager(thread::Pool& threads);

    ~Manager();

    file::System& filesystem();

    thread::Pool& threads();

    void clear();

    void increment_generation();

    template <typename T>
    std::vector<T*> const& register_provider(Provider<T>& provider);

    template <typename T>
    Resource_ptr<T> load(std::string const& filename, Variants const& options = Variants());

    template <typename T>
    Resource_ptr<T> load(std::string const& filename, Variants const& options,
                         std::string& resolved_name);

    template <typename T>
    Resource_ptr<T> load(std::string const& name, void const* data,
                         std::string const& source_name = "", Variants const& options = Variants());

    template <typename T>
    Resource_ptr<T> get(std::string const& filename, Variants const& options = Variants());

    template <typename T>
    Resource_ptr<T> get(uint32_t id) const;

    template <typename T>
    Resource_ptr<T> store(T* resource);

    template <typename T>
    Resource_ptr<T> store(std::string const& name, T* resource,
                          Variants const& options = Variants());

    template <typename T>
    size_t num_bytes() const;

  private:
    template <typename T>
    Typed_cache<T> const* typed_cache() const;

    template <typename T>
    Typed_cache<T>* typed_cache();

    file::System filesystem_;

    thread::Pool& threads_;

    std::map<std::string, Cache*> caches_;
};

}  // namespace resource

#endif
