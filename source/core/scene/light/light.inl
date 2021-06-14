#ifndef SU_SCENE_LIGHT_LIGHT_INL
#define SU_SCENE_LIGHT_LIGHT_INL

#include "base/math/vector3.inl"
#include "light.hpp"
#include "scene/composed_transformation.inl"
#include "scene/scene.hpp"
#include "scene/shape/shape.hpp"

namespace scene::light {

using Transformation = Composed_transformation;

inline bool Light::is_two_sided() const {
    return two_sided_;
}

inline float Light::area() const {
    return extent_;
}

inline Transformation const& Light::transformation_at(uint64_t time, Transformation& trafo,
                                                      Scene const& scene) const {
    return scene.prop_transformation_at(prop_, time, trafo);
}

inline bool Light::is_finite(Scene const& scene) const {
    return scene.prop_shape(prop_)->is_finite();
}

inline bool Light::equals(uint32_t prop, uint32_t part) const {
    return prop_ == prop && part_ == part;
}

inline bool Light::is_light(uint32_t id) {
    return Null != id;
}

inline bool Light::is_area_light(uint32_t id) {
    return 0 == (id & Volume_light_mask);
}

inline uint32_t Light::strip_mask(uint32_t id) {
    return ~Volume_light_mask & id;
}

}  // namespace scene::light

#endif
