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

class Cache {
  public:
    Cache();

    virtual ~Cache();

    void increment_generation();

  protected:
    uint32_t generation_;
};

template <typename T>
class Typed_cache final : public Cache {
  public:
    using Variants = memory::Variant_map;

    Typed_cache(Provider<T>& provider);

    ~Typed_cache() final;

    std::vector<T*> const& resources() const;

    uint32_t load(std::string const& filename, Variants const& options, Manager& resources);

    uint32_t load(std::string const& filename, Variants const& options, Manager& resources,
                         std::string& resolved_name);

    uint32_t load(std::string const& name, void const* data, std::string const& source_name,
                         Variants const& options, Manager& resources);

    uint32_t get(std::string const& filename, Variants const& options);

    T* get(uint32_t id) const;

    uint32_t store(T* resource);

    uint32_t store(std::string const& name, Variants const& options, T* resource);

  private:
    struct Entry {
        uint32_t id;

        uint32_t generation;

        std::string source_name;

        std::filesystem::file_time_type last_write;
    };

    bool check_up_to_date(Entry& entry) const;

    Provider<T>& provider_;

    using Key = std::pair<std::string, memory::Variant_map>;

    std::vector<T*> resources_;

    std::map<Key, Entry> entries_;
};

}  // namespace resource

#endif
