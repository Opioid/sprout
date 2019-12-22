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
    using Variant_map = memory::Variant_map;

    Manager(thread::Pool& threads) noexcept;

    ~Manager() noexcept;

    file::System& filesystem() noexcept;

    thread::Pool& threads() noexcept;

    void increment_generation() noexcept;

    template <typename T>
    std::vector<T*> const& register_provider(Provider<T>& provider) noexcept;

    template <typename T>
    Resource_ptr<T> load(std::string const& filename,
                         Variant_map const& options = Variant_map()) noexcept;

    template <typename T>
    Resource_ptr<T> load(std::string const& filename, Variant_map const& options,
                         std::string& resolved_name) noexcept;

    template <typename T>
    Resource_ptr<T> load(std::string const& name, void const* data, std::string const& source_name = "",
                         Variant_map const& options = Variant_map()) noexcept;

    template <typename T>
    Resource_ptr<T> get(std::string const& filename,
                        Variant_map const& options = Variant_map()) noexcept;

	template <typename T>
    Resource_ptr<T> get(uint32_t id) const noexcept;

    template <typename T>
    Resource_ptr<T> store(T* resource) noexcept;

    template <typename T>
    Resource_ptr<T> store(std::string const& name, T* resource,
                          Variant_map const& options = Variant_map()) noexcept;

    template <typename T>
    size_t num_bytes() const noexcept;

  private:
    template <typename T>
    Typed_cache<T> const* typed_cache() const noexcept;

    template <typename T>
    Typed_cache<T>* typed_cache() noexcept;

    file::System filesystem_;

    thread::Pool& threads_;

    std::map<std::string, Cache*> caches_;
};

}  // namespace resource

#endif
