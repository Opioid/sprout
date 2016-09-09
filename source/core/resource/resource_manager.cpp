#include "resource_manager.hpp"
#include "resource_cache.hpp"

namespace resource {

Manager::Manager(file::System& file_system, thread::Pool& thread_pool) :
	file_system_(file_system),
	thread_pool_(thread_pool) {}

Manager::~Manager() {
	for (auto c : caches_) {
		delete c.second;
	}
}

file::System& Manager::file_system() {
	return file_system_;
}

thread::Pool& Manager::thread_pool() {
	return thread_pool_;
}

}
