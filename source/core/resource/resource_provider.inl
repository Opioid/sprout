#ifndef SU_CORE_RESOURCE_PROVIDER_INL
#define SU_CORE_RESOURCE_PROVIDER_INL

#include "resource_provider.hpp"

namespace resource {

template<typename T>
Provider<T>::Provider(const std::string& name) : Identifiable<T>(name) {}

template<typename T>
Provider<T>::~Provider() {}

}

#endif
