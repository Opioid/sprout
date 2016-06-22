#pragma once

#include "resource_cache.hpp"
#include "file/file_system.hpp"
#include "base/memory/variant_map.inl"

namespace resource {

template<typename T>
Typed_cache<T>::Typed_cache(Provider<T>& provider) : provider_(provider) {}

template<typename T>
std::shared_ptr<T> Typed_cache<T>::load(const std::string& filename,
										const memory::Variant_map& options,
										Manager& manager, bool& was_cached) {
	auto key = std::make_pair(filename, options);

	auto cached = resources_.find(key);
	if (resources_.end() != cached) {
		was_cached = true;
		return cached->second;
	}

	was_cached = false;

	auto resource = provider_.load(filename, options, manager);
	if (!resource) {
		return nullptr;
	}

	resources_[key] = resource;

	return resource;
}

template<typename T>
std::shared_ptr<T> Typed_cache<T>::get(const std::string& filename,
									   const memory::Variant_map& options) {
	auto key = std::make_pair(filename, options);

	auto cached = resources_.find(key);
	if (resources_.end() != cached) {
		return cached->second;
	}

	return nullptr;
}

}
