#pragma once

#include "resource_provider.hpp"
#include "file/file_system.hpp"

namespace resource {

template<typename T>
Provider<T>::Provider(const std::string& name) : Identifiable<T>(name) {}

template<typename T>
Provider<T>::~Provider() {}

}
