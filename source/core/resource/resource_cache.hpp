#pragma once

#include "resource_provider.hpp"
#include <string>
#include <memory>
#include <map>
#include <utility>

namespace resource {

template<typename T>
class Cache {
public:

	Cache(Provider<T>& provider);

	std::shared_ptr<T> load(const std::string& filename, const memory::Variant_map& options, bool& was_cached);

private:

	Provider<T>& provider_;

	std::map<std::pair<std::string, memory::Variant_map>, std::shared_ptr<T>> resources_;
};

}
