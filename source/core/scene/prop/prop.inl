#ifndef SU_CORE_SCENE_PROP_PROP_INL
#define SU_CORE_SCENE_PROP_PROP_INL

#include "prop.hpp"

namespace scene::prop {

inline uint32_t Prop::shape() const noexcept {
    return shape_;
}

}  // namespace scene::prop

#endif
