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

	file::System& file_system();
	thread::Pool& thread_pool();

	template<typename Type>
	void register_provider(Provider<Type>& provider);

	template<typename Type>
	std::shared_ptr<Type> load(const std::string& filename, const memory::Variant_map& options);

	template<typename Type>
	std::shared_ptr<Type> load(const std::string& filename, const memory::Variant_map& options,
							   bool& was_cached);

	template<typename Type>
	std::shared_ptr<Type> get(const std::string& filename, const memory::Variant_map& options);

private:

	template<typename Type>
	Typed_cache<Type>* typed_cache();

	file::System& file_system_;
	thread::Pool& thread_pool_;

	std::map<uint32_t, Cache*> caches_;
};

}
