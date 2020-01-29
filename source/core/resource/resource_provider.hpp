#ifndef SU_CORE_RESOURCE_PROVIDER_HPP
#define SU_CORE_RESOURCE_PROVIDER_HPP

#include <string>

namespace memory {
class Variant_map;
}

namespace resource {

class Manager;

template <typename T>
class Provider {
  public:
    using Variants  = memory::Variant_map;
    using Resources = Manager;

    Provider() noexcept;

    virtual ~Provider() noexcept;

    virtual T* load(std::string const& filename, Variants const& options, Resources& resources,
                    std::string& resolved_name) noexcept = 0;

    virtual T* load(void const* data, std::string const& source_name, Variants const& options,
                    Resources& resources) noexcept = 0;

    virtual size_t num_bytes() const noexcept = 0;

    virtual size_t num_bytes(T const* resource) const noexcept = 0;
};

}  // namespace resource

#endif
