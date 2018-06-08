#include "plane.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Plane::Plane() { aabb_.set_min_max(float3::identity(), float3::identity()); }

bool Plane::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                      Intersection& intersection) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    intersection.epsilon = 5e-4f * hit_t;

    float3 p = ray.point(hit_t);
    float3 t = -transformation.rotation.r[0];
    float3 b = -transformation.rotation.r[1];

    intersection.p = p;
    intersection.t = t;
    intersection.b = b;
    intersection.n = normal;
    intersection.geo_n = normal;
    intersection.uv[0] = math::dot(t, p) * transformation.scale[0];
    intersection.uv[1] = math::dot(b, p) * transformation.scale[1];

    intersection.part = 0;

    ray.max_t = hit_t;
    return true;
  }

  return false;
}

bool Plane::intersect_fast(Ray& ray, Transformation const& transformation,
                           Node_stack& /*node_stack*/, Intersection& intersection) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    intersection.epsilon = 5e-4f * hit_t;

    float3 p = ray.point(hit_t);
    float3 t = -transformation.rotation.r[0];
    float3 b = -transformation.rotation.r[1];

    intersection.p = p;
    intersection.geo_n = normal;
    intersection.uv[0] = math::dot(t, p) * transformation.scale[0];
    intersection.uv[1] = math::dot(b, p) * transformation.scale[1];

    intersection.part = 0;

    ray.max_t = hit_t;
    return true;
  }

  return false;
}

bool Plane::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                      float& epsilon) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    ray.max_t = hit_t;
    epsilon = 5e-4f * hit_t;
    return true;
  }

  return false;
}

bool Plane::intersect_p(Ray const& ray, Transformation const& transformation,
                        Node_stack& /*node_stack*/) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    return true;
  }

  return false;
}

float Plane::opacity(Ray const& ray, Transformation const& transformation,
                     Materials const& materials, Sampler_filter filter,
                     Worker const& worker) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    float3 p = ray.point(hit_t);
    float2 uv(math::dot(transformation.rotation.r[0], p),
              math::dot(transformation.rotation.r[1], p));

    return materials[0]->opacity(uv, ray.time, filter, worker);
  }

  return 0.f;
}

float3 Plane::thin_absorption(Ray const& ray, Transformation const& transformation,
                              Materials const& materials, Sampler_filter filter,
                              Worker const& worker) const {
  float3 const& normal = transformation.rotation.r[2];
  float d = math::dot(normal, transformation.position);
  float denom = -math::dot(normal, ray.direction);
  float numer = math::dot(normal, ray.origin) - d;
  float hit_t = numer / denom;

  if (hit_t > ray.min_t && hit_t < ray.max_t) {
    float3 p = ray.point(hit_t);
    float2 uv(math::dot(transformation.rotation.r[0], p),
              math::dot(transformation.rotation.r[1], p));

    return materials[0]->thin_absorption(ray.direction, normal, uv, ray.time, filter, worker);
  }

  return float3(0.f);
}

bool Plane::sample(uint32_t /*part*/, f_float3 /*p*/, f_float3 /*n*/,
                   Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                   sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                   Node_stack& /*node_stack*/, Sample& /*sample*/) const {
  return false;
}

bool Plane::sample(uint32_t /*part*/, f_float3 /*p*/, Transformation const& /*transformation*/,
                   float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                   uint32_t /*sampler_dimension*/, Node_stack& /*node_stack*/,
                   Sample& /*sample*/) const {
  return false;
}

float Plane::pdf(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
                 Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                 bool /*total_sphere*/) const {
  return 0.f;
}

bool Plane::sample(uint32_t /*part*/, f_float3 /*p*/, float2 /*uv*/,
                   Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                   Sample& sample) const {
  sample.pdf = 0.f;
  return false;
}

float Plane::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                    Transformation const& /*transformation*/, float /*area*/,
                    bool /*two_sided*/) const {
  return 0.f;
}

float Plane::uv_weight(float2 /*uv*/) const { return 1.f; }

float Plane::area(uint32_t /*part*/, f_float3 /*scale*/) const { return 1.f; }

bool Plane::is_finite() const { return false; }

size_t Plane::num_bytes() const { return sizeof(*this); }

}  // namespace scene::shape
