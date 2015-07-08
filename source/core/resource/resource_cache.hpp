#pragma once

#include "resource_provider.hpp"
#include <string>
#include <memory>
#include <map>
#include <utility>

namespace file {

class System;

}

namespace resource {

template<typename T>
class Cache {
public:

	Cache(file::System& file_system, Provider<T>& provider);

	std::shared_ptr<T> load(const std::string& filename, uint32_t flags = 0);

private:

	file::System& file_system_;
	Provider<T>& provider_;

	std::map<std::pair<std::string, uint32_t>, std::shared_ptr<T>> resources_;
};

}
