#pragma once

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

	std::shared_ptr<T> load(const std::string& filename, const memory::Variant_map& options,
							Manager& manager, bool& was_cached);

	std::shared_ptr<T> load(const std::string& name, const void* data,
							const std::string& mount_folder,
							const memory::Variant_map& options, Manager& manager);

	std::shared_ptr<T> get(const std::string& filename, const memory::Variant_map& options);

	size_t num_bytes() const;

private:

	Provider<T>& provider_;

	std::map<std::pair<std::string, memory::Variant_map>, std::shared_ptr<T>> resources_;
};

}
