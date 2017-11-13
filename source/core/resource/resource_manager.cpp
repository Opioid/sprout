#include "resource_manager.hpp"
#include "resource_cache.hpp"

namespace resource {

Manager::Manager(file::System& filesystem, thread::Pool& thread_pool) :
	filesystem_(filesystem),
	thread_pool_(thread_pool) {}

Manager::~Manager() {
	for (auto c : caches_) {
		delete c.second;
	}
}

file::System& Manager::filesystem() {
	return filesystem_;
}

thread::Pool& Manager::thread_pool() {
	return thread_pool_;
}

}
