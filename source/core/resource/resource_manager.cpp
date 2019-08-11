#include "resource_manager.hpp"
#include "resource_cache.hpp"

namespace resource {

Manager::Manager(file::System& filesystem, thread::Pool& thread_pool) noexcept
    : filesystem_(filesystem), thread_pool_(thread_pool) {}

Manager::~Manager() noexcept {
    for (auto c : caches_) {
        delete c.second;
    }
}

file::System& Manager::filesystem() noexcept {
    return filesystem_;
}

thread::Pool& Manager::thread_pool() noexcept {
    return thread_pool_;
}

void Manager::increment_generation() noexcept {
    for (auto c : caches_) {
        c.second->increment_generation();
    }
}

}  // namespace resource
