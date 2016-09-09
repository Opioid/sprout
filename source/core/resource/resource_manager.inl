#pragma once

#include "resource_manager.hpp"
#include "resource_cache.inl"
#include "resource_provider.inl"

namespace resource {

template<typename Type>
void Manager::register_provider(Provider<Type>& provider) {
	uint32_t id = Provider<Type>::id();
	auto old = caches_.find(id);

	if (caches_.end() != old) {
		delete old->second;
	}

	caches_[id] = new Typed_cache<Type>(provider);
}

template<typename Type>
std::shared_ptr<Type> Manager::load(const std::string& filename,
									const memory::Variant_map& options) {
	bool was_cached;
	return load<Type>(filename, options, was_cached);
}

template<typename Type>
std::shared_ptr<Type> Manager::load(const std::string& filename,
									const memory::Variant_map& options,
									bool& was_cached) {
	if (filename.empty()) {
		return nullptr;
	}

	Typed_cache<Type>* cache = typed_cache<Type>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->load(filename, options, *this, was_cached);
}

template<typename Type>
std::shared_ptr<Type> Manager::load(const std::string& name, const void* data,
									const std::string& mount_folder,
									const memory::Variant_map& options) {
	if (name.empty()) {
		return nullptr;
	}

	Typed_cache<Type>* cache = typed_cache<Type>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->load(name, data, mount_folder, options, *this);
}

template<typename Type>
std::shared_ptr<Type> Manager::get(const std::string& filename,
								   const memory::Variant_map& options) {
	if (filename.empty()) {
		return nullptr;
	}

	Typed_cache<Type>* cache = typed_cache<Type>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->get(filename, options);
}

template<typename Type>
size_t Manager::num_bytes() const {
	const Typed_cache<Type>* cache = typed_cache<Type>();
	if (!cache) {
		return 0;
	}

	return cache->num_bytes();
}

template<typename Type>
const Typed_cache<Type>* Manager::typed_cache() const {
	auto cache = caches_.find(Provider<Type>::id());

	if (caches_.end() == cache) {
		return nullptr;
	}

	return static_cast<const Typed_cache<Type>*>(cache->second);
}

template<typename Type>
Typed_cache<Type>* Manager::typed_cache() {
	auto cache = caches_.find(Provider<Type>::id());

	if (caches_.end() == cache) {
		return nullptr;
	}

	return static_cast<Typed_cache<Type>*>(cache->second);
}

}
