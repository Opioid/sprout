#pragma once

#include <istream>
#include <memory>

namespace memory { class Variant_map; }

namespace file { class System; }

namespace thread { class Pool; }

namespace resource {

template<typename T>
class Provider {
public:

	Provider(file::System& file_system, thread::Pool& thread_pool);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(const std::string& filename, const memory::Variant_map& options) = 0;

protected:

	file::System& file_system_;
	thread::Pool& thread_pool_;
};

}
