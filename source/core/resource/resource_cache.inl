#include "resource_cache.hpp"

namespace resource {

template<typename T>
Cache<T>::Cache(Provider<T>& provider) : provider_(provider) {}

template<typename T>
std::shared_ptr<T> Cache<T>::load(const std::string& filename) {
	auto cached = resources_.find(filename);
	if (resources_.end() != cached) {
		return cached->second;
	}

	auto resource = provider_.load(filename);
	if (!resource) {
		return nullptr;
	}

	resources_[filename] = resource;

	return resource;
}

}
