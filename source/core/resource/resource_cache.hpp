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
    Cache();

    virtual ~Cache();

    void increment_generation();

    bool deprecate_frame_dependant();

  protected:
    struct Entry {
        uint32_t id;
        uint32_t generation;

        std::string source_name;

        std::filesystem::file_time_type last_write;
    };

    bool check_up_to_date(Entry& entry) const;

    uint32_t generation_;

    using Variants = memory::Variant_map;
    using Key      = std::pair<std::string, Variants>;

    std::map<Key, Entry> entries_;
};

template <typename T>
class Typed_cache final : public Cache {
  public:
    Typed_cache(Provider<T>& provider);

    ~Typed_cache() final;

    std::vector<T*> const& resources() const;

    bool reload_frame_dependant(Manager& resources);

    Resource_ptr<T> load(std::string const& filename, Variants const& options, Manager& resources);

    Resource_ptr<T> load(std::string const& filename, Variants const& options, Manager& resources,
                         std::string& resolved_name);

    Resource_ptr<T> load(std::string const& name, void const* const data,
                         std::string const& source_name, Variants const& options,
                         Manager& resources);

    Resource_ptr<T> get(std::string const& name, Variants const& options);

    T* get(uint32_t id) const;

    uint32_t store(T* resource);

    uint32_t store(std::string const& name, Variants const& options, T* resource);

  private:
    Provider<T>& provider_;

    std::vector<T*> resources_;
};

}  // namespace resource

#endif
