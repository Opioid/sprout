#ifndef SU_CORE_RESOURCE_HPP
#define SU_CORE_RESOURCE_HPP

#include <cstdint>

namespace resource {

static uint32_t constexpr Null = 0xFFFFFFFF;

template <typename T>
struct Resource_ptr {
    T* ptr;

    uint32_t id;

    bool operator!() const noexcept {
        return nullptr == ptr;
    }

    static Resource_ptr constexpr Null() noexcept {
        return {nullptr, resource::Null};
    }
};

}  // namespace resource

#endif
