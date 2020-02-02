#ifndef SU_CORE_RESOURCE_HPP
#define SU_CORE_RESOURCE_HPP

#include <cstdint>

namespace resource {

static uint32_t constexpr Null = 0xFFFFFFFF;

template <typename T>
struct Resource_ptr {
    T* ptr;

    uint32_t id;

    bool operator!() const {
        return nullptr == ptr;
    }

    static Resource_ptr constexpr Null() {
        return {nullptr, resource::Null};
    }
};

}  // namespace resource

#endif
