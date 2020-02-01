#include "sphere.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

// Hearn and Baker sphere intersection test as seen
// in "Precision Improvements for Ray/Sphere Intersection"
// and
// https://github.com/NVIDIAGameWorks/GettingStartedWithRTXRayTracing/blob/master/DXR-Sphereflake/Data/Sphereflake/sphereIntersect.hlsli
// Note that the ray direction is not necessarily normalized, which might make problems,
// but so far I didn't encounter any.

namespace scene::shape {

Sphere::Sphere() noexcept = default;

float3 Sphere::object_to_texture_point(float3 const& p) const noexcept {
    return (p - float3(-1.f)) * (1.f / float3(2.f));
}

float3 Sphere::object_to_texture_vector(float3 const& v) const noexcept {
    return v * (1.f / float3(2.f));
}

AABB Sphere::transformed_aabb(float4x4 const& m) const noexcept {
    return AABB(float3(-1.f), float3(1.f)).transform(m);
}

static inline void intersect(float hit_t, Ray const& ray,
                             Shape::Transformation const& transformation,
                             Intersection&                intersection) {
    float3 const p = ray.point(hit_t);
    float3 const n = normalize(p - transformation.position);

    float3 const xyz = normalize(transform_vector_transposed(transformation.rotation, n));

    float const phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
    float const theta = std::acos(xyz[1]);

    // avoid singularity at poles
    float const sin_theta         = std::max(std::sin(theta), 0.00001f);
    auto const [sin_phi, cos_phi] = sincos(phi);

    float3 t(sin_theta * cos_phi, 0.f, sin_theta * sin_phi);
    t = normalize(transform_vector(transformation.rotation, t));

    intersection.p     = p;
    intersection.t     = t;
    intersection.b     = -cross(t, n);
    intersection.n     = n;
    intersection.geo_n = n;
    intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
    intersection.part  = 0;
}

bool Sphere::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       Intersection& intersection) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant >= 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if ((t0 > ray.min_t) & (t0 < ray.max_t)) {
            shape::intersect(t0, ray, transformation, intersection);

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t0;
            return true;
        }

        float const t1 = b + dist;

        if ((t1 > ray.min_t) & (t1 < ray.max_t)) {
            shape::intersect(t1, ray, transformation, intersection);

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t1;
            return true;
        }
    }

    return false;
}

static inline void intersect_nsf(float hit_t, Ray const& ray,
                                 Shape::Transformation const& transformation,
                                 Intersection&                intersection) {
    float3 const p = ray.point(hit_t);
    float3 const n = normalize(p - transformation.position);

    float3 const xyz = normalize(transform_vector_transposed(transformation.rotation, n));

    float const phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
    float const theta = std::acos(xyz[1]);

    intersection.p     = p;
    intersection.geo_n = n;
    intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
    intersection.part  = 0;
}

bool Sphere::intersect_nsf(Ray& ray, Transformation const&           transformation,
                           Node_stack& /*node_stack*/, Intersection& intersection) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if ((t0 > ray.min_t) & (t0 < ray.max_t)) {
            shape::intersect_nsf(t0, ray, transformation, intersection);

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t0;
            return true;
        }

        float const t1 = b + dist;

        if ((t1 > ray.min_t) & (t1 < ray.max_t)) {
            shape::intersect_nsf(t1, ray, transformation, intersection);

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t1;
            return true;
        }
    }

    return false;
}

bool Sphere::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       Normals& normals) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            ray.max_t = t0;

            float3 const p = ray.point(t0);
            float3 const n = normalize(p - transformation.position);

            normals.geo_n = n;
            normals.n     = n;

            return true;
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            ray.max_t = t1;

            float3 const p = ray.point(t1);
            float3 const n = normalize(p - transformation.position);

            normals.geo_n = n;
            normals.n     = n;

            return true;
        }
    }

    return false;
}

bool Sphere::intersect_p(Ray const& ray, Transformation const& transformation,
                         Node_stack& /*node_stack*/) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float const dist = std::sqrt(discriminant);
        float const t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            return true;
        }

        float const t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            return true;
        }
    }

    return false;
}

float Sphere::opacity(Ray const& ray, Transformation const& transformation, uint32_t entity,
                      Filter filter, Worker const& worker) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float dist = std::sqrt(discriminant);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 n = normalize(ray.point(t0) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 n = normalize(ray.point(t1) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
        }
    }

    return 0.f;
}

bool Sphere::thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                             Filter filter, Worker const& worker, float3& ta) const noexcept {
    float3 const v = transformation.position - ray.origin;
    float const  b = dot(ray.direction, v);

    float3 const remedy_term = v - b * ray.direction;

    float const radius       = transformation.scale_x();
    float const discriminant = radius * radius - dot(remedy_term, remedy_term);

    if (discriminant > 0.f) {
        float dist = std::sqrt(discriminant);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 n = normalize(ray.point(t0) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, n, uv,
                                                                          ray.time, filter, worker);
            return true;
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 n = normalize(ray.point(t1) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, n, uv,
                                                                          ray.time, filter, worker);
            return true;
        }
    }

    ta = float3(0.f);
    return true;
}

bool Sphere::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                    float /*area*/, bool /*two_sided*/, Sampler&              sampler,
                    uint32_t sampler_dimension, Node_stack& /*node_stack*/, Sample_to& sample) const
    noexcept {
    float3 const axis = transformation.position - p;

    float const axis_squared_length = squared_length(axis);

    float const radius         = transformation.scale_x();
    float const radius_square  = radius * radius;
    float const sin_theta_max2 = radius_square / axis_squared_length;
    float const cos_theta_max  = std::min(std::sqrt(std::max(0.f, 1.f - sin_theta_max2)),
                                         0.99999995f);

    float const axis_length = std::sqrt(axis_squared_length);

    float3 const z = axis / axis_length;

    auto const [x, y] = orthonormal_basis(z);

    float2 const r2  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

    float const b   = dot(axis, dir);
    float const det = (b * b) - axis_squared_length + radius_square;

    if (det > 0.f) {
        float const dist = std::sqrt(det);
        float const t    = offset_b(b - dist);

        sample.wi  = dir;
        sample.pdf = cone_pdf_uniform(cos_theta_max);
        sample.t   = t;

        return true;
    }

    return false;
}

bool Sphere::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                    bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                    float2       importance_uv, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                    Sample_from& sample) const noexcept {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);
    float3 const ls = sample_sphere_uniform(r0);

    float3 const ws = transformation.position + (transformation.scale_x() * ls);

    auto const [x, y] = orthonormal_basis(ls);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, ls);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / ((1.f * Pi) * area);

    return true;
}

float Sphere::pdf(Ray const&            ray, Intersection const& /*intersection*/,
                  Transformation const& transformation, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/) const noexcept {
    float3 const axis = transformation.position - ray.origin;

    float const axis_squared_length = squared_length(axis);
    float const radius              = transformation.scale_x();
    float const radius_square       = radius * radius;
    float const sin_theta_max2      = radius_square / axis_squared_length;
    float const cos_theta_max       = std::min(std::sqrt(std::max(0.f, 1.f - sin_theta_max2)),
                                         0.99999995f);

    return cone_pdf_uniform(cos_theta_max);
}

float Sphere::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                         Transformation const& /*transformation*/, float /*volume*/) const
    noexcept {
    return 0.f;
}

bool Sphere::sample(uint32_t /*part*/, float3 const& p, float2 uv,
                    Transformation const& transformation, float area, bool /*two_sided*/,
                    Sample_to& sample) const noexcept {
    float phi   = (uv[0] + 0.75f) * (2.f * Pi);
    float theta = uv[1] * Pi;

    float sin_theta = std::sin(theta);
    float cos_theta = std::cos(theta);
    float sin_phi   = std::sin(phi);
    float cos_phi   = std::cos(phi);

    float3 ls(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
    float3 ws = transformation.object_to_world_point(ls);

    float3 axis = ws - p;
    float  sl   = squared_length(axis);
    float  d    = std::sqrt(sl);

    float3 dir = axis / d;

    float3 wn = normalize(ws - transformation.position);

    float c = -dot(wn, dir);

    if (c <= 0.f) {
        return false;
    }

    sample.wi  = dir;
    sample.uvw = float3(uv);
    sample.t   = offset_b(d);
    // sin_theta because of the uv weight
    sample.pdf = sl / (c * area * sin_theta);

    return true;
}

bool Sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                    Transformation const& /*transformation*/, float /*volume*/,
                    Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Sphere::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                    float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                    AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Sphere::pdf_uv(Ray const& ray, Intersection const&             intersection,
                     Transformation const& /*transformation*/, float area, bool /*two_sided*/) const
    noexcept {
    //	float3 xyz = transform_vector_transposed(wn, transformation.rotation);
    //	uv[0] = -std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    //	uv[1] =  std::acos(xyz[1]) * Pi_inv;

    //	// sin_theta because of the uv weight
    //	float sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(intersection.uv[1] * Pi);

    float const sl = ray.max_t * ray.max_t;
    float const c  = -dot(intersection.geo_n, ray.direction);
    return sl / (c * area * sin_theta);
}

float Sphere::uv_weight(float2 uv) const noexcept {
    float const sin_theta = std::sin(uv[1] * Pi);

    if (0.f == sin_theta) {
        // this case never seemed to be an issue?!
        return 0.f;
    }

    return 1.f / sin_theta;
}

float Sphere::area(uint32_t /*part*/, float3 const& scale) const noexcept {
    return (4.f * Pi) * (scale[0] * scale[0]);
}

float Sphere::volume(uint32_t /*part*/, float3 const& scale) const noexcept {
    return ((4.f / 3.f) * Pi) * (scale[0] * scale[0] * scale[0]);
}

size_t Sphere::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
