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

	Provider(const std::string& name);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(const std::string& filename,
									const memory::Variant_map& options,
									Manager& manager) = 0;

	virtual std::shared_ptr<T> load(const void* data,
									const std::string& mount_folder,
									const memory::Variant_map& options,
									Manager& manager) = 0;

	virtual size_t num_bytes() const = 0;
};

}

#endif
