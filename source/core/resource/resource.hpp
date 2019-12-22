#ifndef SU_CORE_RESOURCE_HPP
#define SU_CORE_RESOURCE_HPP

#include <cstdint>

namespace resource {

template <typename T>
struct Resource_ptr {
    T*       ptr;
    uint32_t id;

	bool operator!() const noexcept {
        return nullptr == ptr;
	}

    static Resource_ptr constexpr Null() noexcept {
        return {nullptr, 0xFFFFFFFF};
    }
};

}  // namespace resource

#endif
