#ifndef SU_CORE_RESOURCE_PROVIDER_HPP
#define SU_CORE_RESOURCE_PROVIDER_HPP

#include "identifiable.hpp"
#include <memory>

namespace memory { class Variant_map; }

namespace resource {

class Manager;

template<typename T>
class Provider : public Identifiable<T> {

public:

	Provider(std::string const& name);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(std::string const& filename,
									memory::Variant_map const& options,
									Manager& manager) = 0;

	virtual std::shared_ptr<T> load(void const* data,
									std::string const& mount_folder,
									memory::Variant_map const& options,
									Manager& manager) = 0;

	virtual size_t num_bytes() const = 0;
};

}

#endif
