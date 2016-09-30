#pragma once

#include "resource_manager.hpp"
#include "resource_cache.inl"
#include "resource_provider.inl"

namespace resource {

template<typename T>
void Manager::register_provider(Provider<T>& provider) {
	uint32_t id = Provider<T>::id();
	auto old = caches_.find(id);

	if (caches_.end() != old) {
		delete old->second;
	}

	caches_[id] = new Typed_cache<T>(provider);
}

template<typename T>
std::shared_ptr<T> Manager::load(const std::string& filename,
								 const memory::Variant_map& options) {
	bool was_cached;
	return load<T>(filename, options, was_cached);
}

template<typename T>
std::shared_ptr<T> Manager::load(const std::string& filename,
								 const memory::Variant_map& options,
								 bool& was_cached) {
	if (filename.empty()) {
		return nullptr;
	}

	Typed_cache<T>* cache = typed_cache<T>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->load(filename, options, *this, was_cached);
}

template<typename T>
std::shared_ptr<T> Manager::load(const std::string& name, const void* data,
								 const std::string& mount_folder,
								 const memory::Variant_map& options) {
	if (name.empty()) {
		return nullptr;
	}

	Typed_cache<T>* cache = typed_cache<T>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->load(name, data, mount_folder, options, *this);
}

template<typename T>
std::shared_ptr<T> Manager::get(const std::string& filename,
								const memory::Variant_map& options) {
	if (filename.empty()) {
		return nullptr;
	}

	Typed_cache<T>* cache = typed_cache<T>();

	// a provider for this resource type was never registered
	if (!cache) {
		return nullptr;
	}

	return cache->get(filename, options);
}

template<typename T>
void Manager::store(const std::string& name, const memory::Variant_map& options,
					std::shared_ptr<T> resource) {
	if (name.empty() || !resource) {
		return;
	}

	Typed_cache<T>* cache = typed_cache<T>();

	// a provider for this resource type was never registered
	if (!cache) {
		return;
	}

	return cache->store(name, options, resource);
}

template<typename T>
size_t Manager::num_bytes() const {
	const Typed_cache<T>* cache = typed_cache<T>();
	if (!cache) {
		return 0;
	}

	return cache->num_bytes();
}

template<typename T>
const Typed_cache<T>* Manager::typed_cache() const {
	auto cache = caches_.find(Provider<T>::id());

	if (caches_.end() == cache) {
		return nullptr;
	}

	return static_cast<const Typed_cache<T>*>(cache->second);
}

template<typename T>
Typed_cache<T>* Manager::typed_cache() {
	auto cache = caches_.find(Provider<T>::id());

	if (caches_.end() == cache) {
		return nullptr;
	}

	return static_cast<Typed_cache<T>*>(cache->second);
}

}
