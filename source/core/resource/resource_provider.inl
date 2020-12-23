#ifndef SU_CORE_RESOURCE_PROVIDER_INL
#define SU_CORE_RESOURCE_PROVIDER_INL

#include "resource_provider.hpp"

namespace resource {

template <typename T>
Provider<T>::Provider() = default;

template <typename T>
Provider<T>::~Provider() = default;

template <typename T>
void Provider<T>::increment_generation() {}

}  // namespace resource

#endif
