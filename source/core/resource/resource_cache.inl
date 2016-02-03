#pragma once

#include "resource_cache.hpp"
#include "file/file_system.hpp"
#include "base/memory/variant_map.inl"

namespace resource {

template<typename T>
Cache<T>::Cache(Provider<T>& provider) : provider_(provider) {}

template<typename T>
std::shared_ptr<T> Cache<T>::load(const std::string& filename, const memory::Variant_map& options, bool& was_cached) {
	auto key = std::make_pair(filename, options);

	auto cached = resources_.find(key);
	if (resources_.end() != cached) {
		was_cached = true;
		return cached->second;
	}

	was_cached = false;

	auto resource = provider_.load(filename, options);
	if (!resource) {
		return nullptr;
	}

	resources_[key] = resource;

	return resource;
}

}
