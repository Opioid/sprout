#ifndef SU_CORE_RESOURCE_CACHE_HPP
#define SU_CORE_RESOURCE_CACHE_HPP

#include "resource_provider.hpp"
#include <string>
#include <memory>
#include <map>
#include <utility>

namespace resource {

class Cache {

public:

	virtual ~Cache();
};

template<typename T>
class Typed_cache : public Cache {

public:

	Typed_cache(Provider<T>& provider);
	~Typed_cache();

	std::shared_ptr<T> load(std::string const& filename, memory::Variant_map const& options,
							Manager& manager);

	std::shared_ptr<T> load(std::string const& name, void const* data,
							std::string const& mount_folder,
							memory::Variant_map const& options, Manager& manager);

	std::shared_ptr<T> get(std::string const& filename, memory::Variant_map const& options);

	void store(std::string const& name, memory::Variant_map const& options,
			   std::shared_ptr<T> resource);

	size_t num_bytes() const;

private:

	Provider<T>& provider_;

	std::map<std::pair<std::string, memory::Variant_map>, std::shared_ptr<T>> resources_;
};

}

#endif
