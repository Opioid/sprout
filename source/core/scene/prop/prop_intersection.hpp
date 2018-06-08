#ifndef SU_CORE_SCENE_PROP_INTERSECTION_HPP
#define SU_CORE_SCENE_PROP_INTERSECTION_HPP

#include "scene/material/sampler_settings.hpp"
#include "scene/shape/shape_intersection.hpp"

namespace sampler {
class Sampler;
}

namespace scene {

struct Ray;
class Worker;

namespace material {

class Sample;
class Material;

}  // namespace material

namespace prop {

class Prop;

struct Intersection {
  using Sampler_filter = material::Sampler_settings::Filter;

  bool hit() const;

  material::Material* material() const;

  bool is_light() const;

  uint32_t light_id() const;

  float area() const;

  float opacity(float time, Sampler_filter filter, Worker const& worker) const;

  float3 thin_absorption(f_float3 wo, float const time, Sampler_filter filter,
                         Worker& worker) const;

  const material::Sample& sample(f_float3 wo, Ray const& ray, Sampler_filter filter,
                                 sampler::Sampler& sampler, Worker& worker) const;

  bool same_hemisphere(f_float3 v) const;

  Prop const* prop;
  shape::Intersection geo;
};

}  // namespace prop
}  // namespace scene

#endif
