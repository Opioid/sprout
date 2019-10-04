#ifndef SU_CORE_RESOURCE_CACHE_HPP
#define SU_CORE_RESOURCE_CACHE_HPP

#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace memory {
class Variant_map;
}

namespace resource {

class Manager;

template <typename T>
class Provider;

template <typename T>
struct Resource_ptr;

class Cache {
  public:
    Cache() noexcept;

    virtual ~Cache() noexcept;

    void increment_generation() noexcept;

  protected:
    uint32_t generation_;
};

template <typename T>
class Typed_cache : public Cache {
  public:
    Typed_cache(Provider<T>& provider) noexcept;

    ~Typed_cache() noexcept override final;

    std::vector<T*> const& resources() const noexcept;

    Resource_ptr<T> load(std::string const& filename, memory::Variant_map const& options,
                         Manager& manager) noexcept;

    Resource_ptr<T> load(std::string const& filename, memory::Variant_map const& options,
                         Manager& manager, std::string& resolved_name) noexcept;

    Resource_ptr<T> load(std::string const& name, void const* data, std::string const& source_name,
                         memory::Variant_map const& options, Manager& manager) noexcept;

    Resource_ptr<T> get(std::string const& filename, memory::Variant_map const& options) noexcept;

    Resource_ptr<T> store(T* resource) noexcept;

    Resource_ptr<T> store(std::string const& name, memory::Variant_map const& options,
                          T* resource) noexcept;

    size_t num_bytes() const noexcept;

  private:
    struct Entry {
        uint32_t id;

        uint32_t generation;

        std::string source_name;

        std::filesystem::file_time_type last_write;
    };

    bool is_up_to_date(Entry const& entry) const noexcept;

    Provider<T>& provider_;

    using Key = std::pair<std::string, memory::Variant_map>;

    std::vector<T*> resources_;

    std::map<Key, Entry> entries_;
};

}  // namespace resource

#endif
