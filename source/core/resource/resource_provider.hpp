#pragma once

#include <string>
#include <memory>

namespace resource {

template<typename T>
class Provider {
public:

	virtual ~Provider() {}

	virtual std::shared_ptr<T> load(const std::string& filename, uint32_t flags) = 0;
};

}
