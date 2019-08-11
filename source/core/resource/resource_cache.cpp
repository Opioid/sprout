#include "resource_cache.hpp"

namespace resource {

Cache::Cache() noexcept : generation_(0) {}

Cache::~Cache() noexcept {}

void Cache::increment_generation() noexcept {
    ++generation_;
}

}  // namespace resource
