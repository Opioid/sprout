#ifndef SU_CORE_RESOURCE_CACHE_HPP
#define SU_CORE_RESOURCE_CACHE_HPP

#include "resource_provider.hpp"

#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
    namespace filesystem = std::experimental::filesystem;
}
#endif

#include <map>
#include <string>
#include <utility>

namespace resource {

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

    T* load(std::string const& filename, memory::Variant_map const& options,
            Manager& manager) noexcept;

    T* load(std::string const& filename, memory::Variant_map const& options, Manager& manager,
            std::string& resolved_name) noexcept;

    T* load(std::string const& name, void const* data, std::string const& source_name,
            memory::Variant_map const& options, Manager& manager) noexcept;

    T* get(std::string const& filename, memory::Variant_map const& options) noexcept;

    void store(std::string const& name, memory::Variant_map const& options, T* resource) noexcept;

    size_t num_bytes() const noexcept;

  private:
    struct Entry {
        T* data;

        std::string source_name;

        uint32_t generation;

        std::filesystem::file_time_type last_write;
    };

    bool is_up_to_date(Entry const& entry) const noexcept;

    Provider<T>& provider_;

    std::map<std::pair<std::string, memory::Variant_map>, Entry> resources_;
};

}  // namespace resource

#endif
