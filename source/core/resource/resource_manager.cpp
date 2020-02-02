#include "resource_manager.hpp"
#include "resource_cache.hpp"

namespace resource {

Manager::Manager(thread::Pool& threads) : threads_(threads) {}

Manager::~Manager() {
    for (auto const& c : caches_) {
        delete c.second;
    }
}

file::System& Manager::filesystem() {
    return filesystem_;
}

thread::Pool& Manager::threads() {
    return threads_;
}

void Manager::increment_generation() {
    for (auto& c : caches_) {
        c.second->increment_generation();
    }
}

}  // namespace resource
