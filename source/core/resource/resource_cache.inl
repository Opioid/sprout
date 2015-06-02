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

/*
		std::ifstream stream(filename, std::ios::binary);
		if (!stream) {
		//	throw std::runtime_error("File \"" + filename + "\" could not be opened");
			return nullptr;
		}

/*		file::Type type = file::query_type(stream);

		switch (type) {
		case file::Type::GZIP: std::cout << "GZIP" << std::endl; break;
		case file::Type::PNG: std::cout << "PNG" << std::endl; break;
		case file::Type::RGBE: std::cout << "RGBE" << std::endl; break;
		default: std::cout << "Unknown" << std::endl;
		}
*/

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
