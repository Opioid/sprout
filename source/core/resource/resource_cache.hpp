#pragma once

#include "resource_provider.hpp"
#include <string>
#include <memory>
#include <map>

namespace resource {

template<typename T>
class Cache {
public:

	Cache(Provider<T>& provider);

	std::shared_ptr<T> load(const std::string& filename);

private:

	Provider<T>& provider_;

	std::map<std::string, std::shared_ptr<T>> resources_;
};

}
