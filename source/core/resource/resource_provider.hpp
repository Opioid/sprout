#ifndef SU_CORE_RESOURCE_PROVIDER_HPP
#define SU_CORE_RESOURCE_PROVIDER_HPP

#include <memory>
#include "identifiable.hpp"

namespace memory {
class Variant_map;
}

namespace resource {

class Manager;

template <typename T>
class Provider : public Identifiable<T> {
  public:
    using Variant_map = memory::Variant_map;

    Provider(std::string const& name) noexcept;

    virtual ~Provider() noexcept;

    virtual std::shared_ptr<T> load(std::string const& filename, Variant_map const& options,
                                    Manager& manager) = 0;

    virtual std::shared_ptr<T> load(void const* data, std::string_view mount_folder,
                                    Variant_map const& options, Manager& manager) = 0;

    virtual size_t num_bytes() const noexcept = 0;
};

}  // namespace resource

#endif
