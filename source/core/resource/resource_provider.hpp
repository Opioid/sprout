#pragma once

#include <istream>
#include <memory>

namespace file { class System; }

namespace thread { class Pool; }

namespace resource {

template<typename T>
class Provider {
public:

	Provider(file::System& file_system, thread::Pool& thread_pool);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(const std::string& filename, uint32_t flags = 0) = 0;

protected:

	file::System& file_system_;
	thread::Pool& thread_pool_;
};

}
