#pragma once

#include <istream>
#include <memory>

namespace file {

class System;

}

namespace resource {

template<typename T>
class Provider {
public:

	Provider(file::System& file_system);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(const std::string& filename, uint32_t flags = 0) = 0;

protected:

	file::System& file_system_;
};

}
