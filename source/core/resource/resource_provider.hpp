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
    using Variant_map = memory::Variant_map;

    Provider() noexcept;

    virtual ~Provider() noexcept;

    virtual T* load(std::string const& filename, Variant_map const& options,
                    Manager& manager) noexcept = 0;

    virtual T* load(void const* data, std::string_view mount_folder, Variant_map const& options,
                    Manager& manager) noexcept = 0;

    virtual size_t num_bytes() const noexcept = 0;

    virtual size_t num_bytes(T const* resource) const noexcept = 0;
};

}  // namespace resource

#endif
