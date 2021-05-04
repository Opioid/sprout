#include "resource_cache.hpp"
#include "base/memory/variant_map.inl"
#include "file/file_system.hpp"

namespace resource {

Cache::Cache() : generation_(0) {}

Cache::~Cache() = default;

void Cache::increment_generation() {
    ++generation_;
}

bool Cache::deprecate_frame_dependant() {
    bool deprecated = false;

    for (auto& kv : entries_) {
        if (std::string const& filename = kv.first.first;
            file::System::frame_dependant_name(filename)) {
            kv.second.generation = 0xFFFFFFFF;
            deprecated           = true;
        }
    }

    return deprecated;
}

bool Cache::check_up_to_date(Entry& entry) const {
    if (0xFFFFFFFF == entry.generation) {
        return false;
    }

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
