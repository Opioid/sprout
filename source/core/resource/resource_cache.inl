#include "resource_cache.hpp"
#include "file/file.hpp"

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

	auto stream = file::open_read_stream(filename);
	if (!*stream) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	auto resource = provider_.load(*stream, flags);
	if (!resource) {
		return nullptr;
	}

	resources_[key] = resource;

	return resource;
}

}
