#include "resource_cache.hpp"

namespace resource {

template<typename T>
Cache<T>::Cache(Provider<T>& provider) : provider_(provider) {}

template<typename T>
std::shared_ptr<T> Cache<T>::load(const std::string& filename, uint32_t flags) {
	auto key = std::make_pair(filename, flags);

	auto cached = resources_.find(key);
	if (resources_.end() != cached) {
		return cached->second;
	}

	auto resource = provider_.load(filename, flags);
	if (!resource) {
		return nullptr;
	}

	resources_[key] = resource;

	return resource;
}

}
