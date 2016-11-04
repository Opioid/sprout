#pragma once

#include <map>
#include <memory>

namespace memory { class Variant_map; }

namespace file { class System; }

namespace thread { class Pool; }

namespace resource {

class Cache;

template<typename T> class Typed_cache;

template<typename T> class Provider;

class Manager {

public:

	Manager(file::System& file_system, thread::Pool& thread_pool);
	~Manager();

	file::System& file_system();
	thread::Pool& thread_pool();

	template<typename T>
	void register_provider(Provider<T>& provider);

	template<typename T>
	std::shared_ptr<T> load(const std::string& filename, const memory::Variant_map& options);

	template<typename T>
	std::shared_ptr<T> load(const std::string& filename, const memory::Variant_map& options,
							bool& was_cached);

	template<typename T>
	std::shared_ptr<T> load(const std::string& name, const void* data,
							const std::string& mount_folder,
							const memory::Variant_map& options);

	template<typename T>
	std::shared_ptr<T> get(const std::string& filename, const memory::Variant_map& options);

	template<typename T>
	void store(const std::string& name, const memory::Variant_map& options,
			   std::shared_ptr<T> resource);

	template<typename T>
	size_t num_bytes() const;

private:

	template<typename T>
	const Typed_cache<T>* typed_cache() const;

	template<typename T>
	Typed_cache<T>* typed_cache();

	file::System& file_system_;
	thread::Pool& thread_pool_;

	std::map<uint32_t, Cache*> caches_;
};

}
