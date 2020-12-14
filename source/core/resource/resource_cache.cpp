#include "resource_cache.hpp"

namespace resource {

Cache::Cache() : generation_(0) {}

Cache::~Cache() = default;

void Cache::increment_generation() {
    ++generation_;
}

bool Cache::check_up_to_date(Entry& entry) const {
    if (entry.generation == generation_ || entry.source_name.empty()) {
        return true;
    }

    if (std::filesystem::last_write_time(entry.source_name) == entry.last_write) {
        entry.generation = generation_;
        return true;
    }

    return false;
}

}  // namespace resource
