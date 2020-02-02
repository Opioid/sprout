#include "resource_cache.hpp"

namespace resource {

Cache::Cache() : generation_(0) {}

Cache::~Cache() = default;

void Cache::increment_generation() {
    ++generation_;
}

}  // namespace resource
