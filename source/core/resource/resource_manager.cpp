#include "resource_manager.hpp"
#include "resource_cache.hpp"

namespace resource {

Manager::Manager(thread::Pool& threads) noexcept : threads_(threads) {}

Manager::~Manager() noexcept {
    for (auto const& c : caches_) {
        delete c.second;
    }
}

file::System& Manager::filesystem() noexcept {
    return filesystem_;
}

thread::Pool& Manager::threads() noexcept {
    return threads_;
}

void Manager::increment_generation() noexcept {
    for (auto& c : caches_) {
        c.second->increment_generation();
    }
}

}  // namespace resource
