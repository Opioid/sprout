#include "plane.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Plane::Plane() noexcept = default;

float3 Plane::object_to_texture_point(float3 const& p) const noexcept {
    return p;
}

float3 Plane::object_to_texture_vector(float3 const& v) const noexcept {
    return v;
}

AABB Plane::transformed_aabb(float4x4 const& /*m*/) const noexcept {
    return AABB::empty();
}

bool Plane::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                      Intersection& intersection) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 t = -transformation.rotation.r[0];
        float3 b = -transformation.rotation.r[1];

        intersection.p     = p;
        intersection.t     = t;
        intersection.b     = b;
        intersection.n     = normal;
        intersection.geo_n = normal;
        intersection.uv[0] = dot(t, p) * transformation.scale_x();
        intersection.uv[1] = dot(b, p) * transformation.scale_y();

        intersection.part = 0;

        ray.max_t = hit_t;
        return true;
    }

    return false;
}

bool Plane::intersect_nsf(Ray& ray, Transformation const&           transformation,
                          Node_stack& /*node_stack*/, Intersection& intersection) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 t = -transformation.rotation.r[0];
        float3 b = -transformation.rotation.r[1];

        intersection.p     = p;
        intersection.geo_n = normal;
        intersection.uv[0] = dot(t, p) * transformation.scale_x();
        intersection.uv[1] = dot(b, p) * transformation.scale_y();

        intersection.part = 0;

        ray.max_t = hit_t;
        return true;
    }

    return false;
}

bool Plane::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                      Normals& normals) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        ray.max_t = hit_t;

        normals.geo_n = normal;
        normals.n     = normal;

        return true;
    }

    return false;
}

bool Plane::intersect_p(Ray const& ray, Transformation const& transformation,
                        Node_stack& /*node_stack*/) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    return (hit_t > ray.min_t) & (hit_t < ray.max_t);
}

float Plane::opacity(Ray const& ray, Transformation const& transformation, uint32_t entity,
                     Filter filter, Worker const& worker) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float2 uv(dot(transformation.rotation.r[0], p), dot(transformation.rotation.r[1], p));

        return worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
    }

    return 0.f;
}

bool Plane::thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                            Filter filter, Worker const& worker, float3& ta) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float2 uv(dot(transformation.rotation.r[0], p), dot(transformation.rotation.r[1], p));

        ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, normal, uv,
                                                                      ray.time, filter, worker);
        return true;
    }

    ta = float3(1.f);
    return true;
}

bool Plane::sample(uint32_t /*part*/, float3 const& /*p*/, Transformation const& /*transformation*/,
                   float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                   uint32_t /*sampler_dimension*/, Node_stack& /*node_stack*/,
                   Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Plane::sample(uint32_t /*part*/, Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/, Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                   float2 /*importance_uv*/, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                   Sample_from& /*sample*/) const noexcept {
    return false;
}

float Plane::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                 Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                 bool /*total_sphere*/) const noexcept {
    return 0.f;
}

float Plane::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                        Transformation const& /*transformation*/, float /*volume*/) const noexcept {
    return 0.f;
}

bool Plane::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                   Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                   Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Plane::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                   Transformation const& /*transformation*/, float /*volume*/,
                   Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Plane::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                   float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                   AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Plane::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                    Transformation const& /*transformation*/, float /*area*/,
                    bool /*two_sided*/) const noexcept {
    return 0.f;
}

float Plane::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Plane::area(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 1.f;
}

float Plane::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

bool Plane::is_finite() const noexcept {
    return false;
}

size_t Plane::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
