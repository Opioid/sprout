#ifndef SU_SCENE_LIGHT_LIGHT_INL
#define SU_SCENE_LIGHT_LIGHT_INL

#include "base/math/vector3.inl"
#include "light.hpp"

#include "scene/entity/composed_transformation.inl"
#include "scene/material/material.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.hpp"

#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

using Transformation = entity::Composed_transformation;
using Filter         = material::Sampler_settings::Filter;
using Sample_to      = shape::Sample_to;
using Sample_from    = shape::Sample_from;
using Sampler        = sampler::Sampler;

inline Light::Light(Type type, uint32_t prop, uint32_t part)
    : type_(type), prop_(prop), part_(part), extent_(0.f) {}

inline float Light::area() const {
    return extent_;
}

inline void Light::set_extent(float extent) {
    extent_ = extent;
}

inline Transformation const& Light::transformation_at(uint64_t time, Transformation& transformation,
                                                      Scene const& scene) const {
    return scene.prop_transformation_at(prop_, time, transformation);
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
