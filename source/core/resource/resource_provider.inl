#pragma once

#include "resource_provider.hpp"

namespace resource {

template<typename T>
Provider<T>::Provider(const std::string& name) : Identifiable<T>(name) {}

template<typename T>
Provider<T>::~Provider() {}

}
