#pragma once

#include "resource_provider.hpp"
#include "file/file_system.hpp"

namespace resource {

template<typename T>
Provider<T>::Provider(file::System& file_system, thread::Pool& thread_pool) :
	file_system_(file_system), thread_pool_(thread_pool) {}

template<typename T>
Provider<T>::~Provider() {}

}
