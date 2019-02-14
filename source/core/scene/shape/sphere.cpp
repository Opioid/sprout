#include "sphere.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Sphere::Sphere() noexcept {
    aabb_.set_min_max(float3(-1.f), float3(1.f));
    inv_extent_ = 1.f / aabb_.extent();
}

AABB Sphere::transformed_aabb(float4x4 const& /*m*/, math::Transformation const& t) const noexcept {
    return transformed_aabb(t);
}

AABB Sphere::transformed_aabb(math::Transformation const& t) const noexcept {
    float3 const halfsize(t.scale[0]);
    return AABB(t.position - halfsize, t.position + halfsize);
}

bool Sphere::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       Intersection& intersection) const noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 p = ray.point(t0);
            float3 n = normalize(p - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
            float theta = std::acos(xyz[1]);

            // avoid singularity at poles
            float sin_theta = std::max(std::sin(theta), 0.00001f);
            float sin_phi   = std::sin(phi);
            float cos_phi   = std::cos(phi);

            float3 t(sin_theta * cos_phi, 0.f, sin_theta * sin_phi);
            t = normalize(transform_vector(transformation.rotation, t));

            intersection.p     = p;
            intersection.t     = t;
            intersection.b     = -cross(t, n);
            intersection.n     = n;
            intersection.geo_n = n;
            intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
            intersection.part  = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t0;
            return true;
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 p = ray.point(t1);
            float3 n = normalize(p - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
            float theta = std::acos(xyz[1]);

            // avoid singularity at poles
            float sin_theta = std::max(std::sin(theta), 0.00001f);
            float sin_phi   = std::sin(phi);
            float cos_phi   = std::cos(phi);

            float3 t(sin_theta * cos_phi, 0.f, sin_theta * sin_phi);
            t = normalize(transform_vector(transformation.rotation, t));

            intersection.p     = p;
            intersection.t     = t;
            intersection.b     = -cross(t, n);
            intersection.n     = n;
            intersection.geo_n = n;
            intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
            intersection.part  = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t1;
            return true;
        }
    }

    return false;
}

bool Sphere::intersect_fast(Ray& ray, Transformation const&           transformation,
                            Node_stack& /*node_stack*/, Intersection& intersection) const noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 p = ray.point(t0);
            float3 n = normalize(p - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
            float theta = std::acos(xyz[1]);

            intersection.p     = p;
            intersection.geo_n = n;
            intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
            intersection.part  = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t0;
            return true;
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 p = ray.point(t1);
            float3 n = normalize(p - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float phi   = -std::atan2(xyz[0], xyz[2]) + Pi;
            float theta = std::acos(xyz[1]);

            intersection.p     = p;
            intersection.geo_n = n;
            intersection.uv    = float2(phi * (0.5f * Pi_inv), theta * Pi_inv);
            intersection.part  = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = t1;
            return true;
        }
    }

    return false;
}

bool Sphere::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       float& epsilon) const noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            ray.max_t = t0;
            epsilon   = 5e-4f * t0;
            return true;
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            ray.max_t = t1;
            epsilon   = 5e-4f * t1;
            return true;
        }
    }

    return false;
}

bool Sphere::intersect_p(Ray const& ray, Transformation const& transformation,
                         Node_stack& /*node_stack*/) const noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            return true;
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            return true;
        }
    }

    return false;
}

float Sphere::opacity(Ray const& ray, Transformation const& transformation, Materials materials,
                      Filter filter, Worker const& worker) const noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 n = normalize(ray.point(t0) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return materials[0]->opacity(uv, ray.time, filter, worker);
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 n = normalize(ray.point(t1) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return materials[0]->opacity(uv, ray.time, filter, worker);
        }
    }

    return 0.f;
}

float3 Sphere::thin_absorption(Ray const& ray, Transformation const& transformation,
                               Materials materials, Filter filter, Worker const& worker) const
    noexcept {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale[0];
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t && t0 < ray.max_t) {
            float3 n = normalize(ray.point(t0) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return materials[0]->thin_absorption(ray.direction, n, uv, ray.time, filter, worker);
        }

        float t1 = b + dist;

        if (t1 > ray.min_t && t1 < ray.max_t) {
            float3 n = normalize(ray.point(t1) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return materials[0]->thin_absorption(ray.direction, n, uv, ray.time, filter, worker);
        }
    }

    return float3(0.f);
}

bool Sphere::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                    float /*area*/, bool /*two_sided*/, Sampler&              sampler,
                    uint32_t sampler_dimension, Node_stack& /*node_stack*/, Sample_to& sample) const
    noexcept {
    float3 const axis = transformation.position - p;

    float const axis_squared_length = squared_length(axis);

    float const radius         = transformation.scale[0];
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
                    AABB const& /*bounds*/, Node_stack& /*node_stack*/, Sample_from& sample) const
    noexcept {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);
    float3 const ls = math::sample_sphere_uniform(r0);

    float3 const ws = transformation.position + (transformation.scale[0] * ls);

    auto const [x, y] = orthonormal_basis(ls);

    float2 const r1  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_hemisphere_cosine(r1, x, y, ls);

    sample.p   = ws;
    sample.dir = dir;
    sample.pdf = 1.f / ((1.f * Pi) * area);

    return true;
}

float Sphere::pdf(Ray const&            ray, const shape::Intersection& /*intersection*/,
                  Transformation const& transformation, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/) const noexcept {
    float3 const axis = transformation.position - ray.origin;

    float const axis_squared_length = squared_length(axis);
    float const radius_square       = transformation.scale[0] * transformation.scale[0];
    float const sin_theta_max2      = radius_square / axis_squared_length;
    float const cos_theta_max       = std::min(std::sqrt(std::max(0.f, 1.f - sin_theta_max2)),
                                         0.99999995f);

    return math::cone_pdf_uniform(cos_theta_max);
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
    float3 ws = transform_point(transformation.object_to_world, ls);

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
                    float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                    uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                    Sample_from& /*sample*/) const noexcept {
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
