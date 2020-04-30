#ifndef SU_CORE_SCENE_SHAPE_SHAPE_INL
#define SU_CORE_SCENE_SHAPE_SHAPE_INL

#include "shape.hpp"

namespace scene::shape {

inline bool Shape::is_complex() const {
    return properties_.is(Property::Complex);
}

inline bool Shape::is_finite() const {
    return properties_.is(Property::Finite);
}

inline bool Shape::is_analytical() const {
    return properties_.is(Property::Analytical);
}

}  // namespace scene::shape

#endif
