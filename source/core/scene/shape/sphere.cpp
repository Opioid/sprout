#include "sphere.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.inl"
#include "scene/ray_offset.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

// Hearn and Baker sphere isec test as seen in "Precision Improvements for Ray/Sphere Intersection",
// and
// https://github.com/NVIDIAGameWorks/GettingStartedWithRTXRayTracing/blob/master/DXR-Sphereflake/Data/Sphereflake/sphereIntersect.hlsli
// Note that the ray direction is not necessarily normalized, which might cause problems,
// but so far I didn't encounter any.

namespace scene::shape {

static float constexpr Delta = 1.e-20f;

Sphere::Sphere() : Shape(Properties(Property::Finite, Property::Analytical)) {}

AABB Sphere::aabb() const {
    return AABB(float3(-1.f), float3(1.f));
}

static inline void intersect(float hit_t, Ray const& ray, Shape::Transformation const& trafo,
                             Intersection& isec) {
    float3 const p   = ray.point(hit_t);
    float3 const n   = normalize(p - trafo.position);
    float3 const xyz = normalize(transform_vector_transposed(trafo.rotation, n));

    float const phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
    float const theta = std::acos(xyz[1]);

    // avoid singularity at poles
    float const sin_theta         = std::max(std::sin(theta), 0.00001f);
    auto const [sin_phi, cos_phi] = sincos(phi);

    float3 t(sin_theta * cos_phi, 0.f, sin_theta * sin_phi);
    t = normalize(transform_vector(trafo.rotation, t));

    isec.p     = p;
    isec.t     = t;
    isec.b     = -cross(t, n);
    isec.n     = n;
    isec.geo_n = n;
    isec.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
    isec.part  = 0;
}

bool Sphere::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                       Intersection& isec) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant >= 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if ((t0 > ray.min_t()) & (t0 < ray.max_t())) {
            shape::intersect(t0, ray, trafo, isec);

            SOFT_ASSERT(testing::check(isec, trafo, ray));

            ray.max_t() = t0;
            return true;
        }

        float const t1 = b + dist;

        if ((t1 > ray.min_t()) & (t1 < ray.max_t())) {
            shape::intersect(t1, ray, trafo, isec);

            SOFT_ASSERT(testing::check(isec, trafo, ray));

            ray.max_t() = t1;
            return true;
        }
    }

    return false;
}

static inline void intersect_nsf(float hit_t, Ray const& ray, Shape::Transformation const& trafo,
                                 Intersection& isec) {
    float3 const p = ray.point(hit_t);

    float3 const n = normalize(p - trafo.position);

    float3 const xyz = normalize(transform_vector_transposed(trafo.rotation, n));

    float const phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
    float const theta = std::acos(xyz[1]);

    isec.p     = p;
    isec.geo_n = n;
    isec.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
    isec.part  = 0;
}

bool Sphere::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                           Intersection& isec) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if ((t0 > ray.min_t()) & (t0 < ray.max_t())) {
            shape::intersect_nsf(t0, ray, trafo, isec);

            SOFT_ASSERT(testing::check(isec, trafo, ray));

            ray.max_t() = t0;
            return true;
        }

        float const t1 = b + dist;

        if ((t1 > ray.min_t()) & (t1 < ray.max_t())) {
            shape::intersect_nsf(t1, ray, trafo, isec);

            SOFT_ASSERT(testing::check(isec, trafo, ray));

            ray.max_t() = t1;
            return true;
        }
    }

    return false;
}

bool Sphere::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                       Normals& normals) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t() && t0 < ray.max_t()) {
            ray.max_t() = t0;

            float3 const p = ray.point(t0);
            float3 const n = normalize(p - trafo.position);

            normals.geo_n = n;
            normals.n     = n;

            return true;
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t() && t1 < ray.max_t()) {
            ray.max_t() = t1;

            float3 const p = ray.point(t1);
            float3 const n = normalize(p - trafo.position);

            normals.geo_n = n;
            normals.n     = n;

            return true;
        }
    }

    return false;
}

bool Sphere::intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& /*nodes*/) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t() && t0 < ray.max_t()) {
            return true;
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t() && t1 < ray.max_t()) {
            return true;
        }
    }

    return false;
}

float Sphere::visibility(Ray const& ray, Transformation const& trafo, uint32_t entity,
                         Filter filter, Worker& worker) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t() && t0 < ray.max_t()) {
            float3 n = normalize(ray.point(t0) - trafo.position);

            float3 xyz = transform_vector_transposed(trafo.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return 1.f - worker.scene().prop_material(entity, 0)->opacity(uv, filter, worker);
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t() && t1 < ray.max_t()) {
            float3 n = normalize(ray.point(t1) - trafo.position);

            float3 xyz = transform_vector_transposed(trafo.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return 1.f - worker.scene().prop_material(entity, 0)->opacity(uv, filter, worker);
        }
    }

    return 1.f;
}

bool Sphere::thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                             Filter filter, Worker& worker, float3& ta) const {
    float3 const v = trafo.position - ray.origin;

    float const b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius = trafo.scale_x();

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t() && t0 < ray.max_t()) {
            float3 n = normalize(ray.point(t0) - trafo.position);

            float3 xyz = transform_vector_transposed(trafo.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, n, uv,
                                                                          filter, worker);
            return true;
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t() && t1 < ray.max_t()) {
            float3 n = normalize(ray.point(t1) - trafo.position);

            float3 xyz = transform_vector_transposed(trafo.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, n, uv,
                                                                          filter, worker);
            return true;
        }
    }

    ta = float3(0.f);
    return true;
}

bool Sphere::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p p, float3_p /*n*/,
                    Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                    bool /*total_sphere*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                    Sample_to& sample) const {
    float3 const v = trafo.position - p;

    float const il            = rlength(v);
    float const radius        = trafo.scale_x();
    float const sin_theta_max = std::min(il * radius, 1.f);
    float const cos_theta_max = std::sqrt(std::max(1.f - sin_theta_max * sin_theta_max, Delta));

    float3 const z = il * v;

    auto const [x, y] = orthonormal_basis(z);

    float2 const r2  = sampler.sample_2D(rng, sampler_d);
    float3 const dir = sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

    float const b = dot(dir, v);

    float3 const remedy_term = v - b * dir;

    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant >= 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t    = b - dist;

        float3 const sp = p + t * dir;
        float3 const sn = normalize(sp - trafo.position);

        sample = Sample_to(dir, sn, float3(0.f), cone_pdf_uniform(cos_theta_max), offset_b(t));

        return true;
    }

    return false;
}

bool Sphere::sample(uint32_t /*part*/, uint32_t /*variant*/, Transformation const& trafo, float area, bool /*two_sided*/,
                    Sampler& sampler, RNG& rng, uint32_t sampler_d, float2 importance_uv,
                    AABB const& /*bounds*/, Sample_from& sample) const {
    float2 const r0 = sampler.sample_2D(rng, sampler_d);
    float3 const ls = sample_sphere_uniform(r0);
    float3 const ws = trafo.object_to_world_point(ls);

    float3 const wn = normalize(ws - trafo.position);

    auto const [x, y] = orthonormal_basis(ls);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, ls);

    sample = Sample_from(offset_ray(ws, wn), wn, dir, float2(0.f), importance_uv,
                         1.f / ((1.f * Pi) * area));

    return true;
}

float Sphere::pdf(uint32_t /*variant*/, Ray const&            ray, float3_p /*n*/, Intersection const& /*isec*/,
                  Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/) const {
    float3 const axis = trafo.position - ray.origin;

    float const il            = rlength(axis);
    float const radius        = trafo.scale_x();
    float const sin_theta_max = std::min(il * radius, 1.f);
    float const cos_theta_max = std::sqrt(std::max(1.f - sin_theta_max * sin_theta_max, Delta));

    return cone_pdf_uniform(cos_theta_max);
}

float Sphere::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                         Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Sphere::sample(uint32_t /*part*/, float3_p p, float2 uv, Transformation const& trafo,
                    float area, bool /*two_sided*/, Sample_to& sample) const {
    float phi   = (uv[0] + 0.75f) * (2.f * Pi);
    float theta = uv[1] * Pi;

    float sin_theta = std::sin(theta);
    float cos_theta = std::cos(theta);
    float sin_phi   = std::sin(phi);
    float cos_phi   = std::cos(phi);

    float3 const ls(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
    float3 const ws = trafo.object_to_world_point(ls);

    float3 axis = ws - p;
    float  sl   = squared_length(axis);
    float  d    = std::sqrt(sl);

    float3 dir = axis / d;

    float3 wn = normalize(ws - trafo.position);

    float c = -dot(wn, dir);

    if (c <= 0.f) {
        return false;
    }

    sample = Sample_to(dir, wn, float3(uv), sl / (c * area * sin_theta), offset_b(d));

    return true;
}

bool Sphere::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                           Transformation const& /*trafo*/, float /*volume*/,
                           Sample_to& /*sample*/) const {
    return false;
}

bool Sphere::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/,
                    float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                    AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Sphere::pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& /*trafo*/,
                     float area, bool /*two_sided*/) const {
    //	float3 xyz = transform_vector_transposed(wn, trafo.rotation);
    //	uv[0] = -std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    //	uv[1] =  std::acos(xyz[1]) * Pi_inv;

    //	// sin_theta because of the uv weight
    //	float sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(isec.uv[1] * Pi);

    float const sl = ray.max_t() * ray.max_t();
    float const c  = -dot(isec.geo_n, ray.direction);
    return sl / (c * area * sin_theta);
}

float Sphere::uv_weight(float2 uv) const {
    float const sin_theta = std::sin(uv[1] * Pi);

    if (0.f == sin_theta) {
        // this case never seemed to be an issue?!
        return 0.f;
    }

    return 1.f / sin_theta;
}

float Sphere::area(uint32_t /*part*/, float3_p scale) const {
    return (4.f * Pi) * (scale[0] * scale[0]);
}

float Sphere::volume(uint32_t /*part*/, float3_p scale) const {
    return ((4.f / 3.f) * Pi) * (scale[0] * scale[0] * scale[0]);
}

Shape::Differential_surface Sphere::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
