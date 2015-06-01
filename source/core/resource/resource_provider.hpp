#pragma once

#include <istream>
#include <memory>

namespace resource {

template<typename T>
class Provider {
public:

	virtual ~Provider() {}

	virtual std::shared_ptr<T> load(std::istream& stream, uint32_t flags) = 0;
};

}
