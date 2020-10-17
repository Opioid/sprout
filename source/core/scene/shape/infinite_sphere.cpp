#include "infinite_sphere.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/sincos.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Infinite_sphere::Infinite_sphere() : Shape(Properties(Property::Analytical)) {}

float3 Infinite_sphere::object_to_texture_point(float3 const& p) const {
    return p;
}

float3 Infinite_sphere::object_to_texture_vector(float3 const& v) const {
    return v;
}

AABB Infinite_sphere::transformed_aabb(float4x4 const& /*m*/) const {
    return AABB::empty();
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                                Intersection& isec) const {
    if (ray.max_t() >= Ray_max_t) {
        // This is nonsense
        isec.t = trafo.rotation.r[0];
        isec.b = trafo.rotation.r[1];

        float3 xyz = transform_vector_transposed(trafo.rotation, ray.direction);
        xyz        = normalize(xyz);

        isec.uv[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
        isec.uv[1] = std::acos(xyz[1]) * Pi_inv;

        isec.p = ray.point(Ray_max_t);

        float3 const n = -ray.direction;

        isec.n     = n;
        isec.geo_n = n;
        isec.part  = 0;

        ray.max_t() = Ray_max_t;

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                                    Intersection& isec) const {
    if (ray.max_t() >= Ray_max_t) {
        float3 xyz = transform_vector_transposed(trafo.rotation, ray.direction);
        xyz        = normalize(xyz);

        isec.uv[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
        isec.uv[1] = std::acos(xyz[1]) * Pi_inv;

        isec.p = ray.point(Ray_max_t);

        float3 const n = -ray.direction;

        isec.geo_n = n;
        isec.part  = 0;

        ray.max_t() = Ray_max_t;

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const& /*trafo*/, Node_stack& /*nodes*/,
                                Normals& normals) const {
    if (ray.max_t() >= Ray_max_t) {
        ray.max_t() = Ray_max_t;

        float3 const n = -ray.direction;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect_p(Ray const& /*ray*/, Transformation const& /*trafo*/,
                                  Node_stack& /*nodes*/) const {
    // Implementation for this is not really needed, so just skip it
    return false;
}

float Infinite_sphere::visibility(Ray const& /*ray*/, Transformation const& /*trafo*/,
                                  uint32_t /*entity*/, Filter /*filter*/,
                                  Worker& /*worker*/) const {
    // Implementation for this is not really needed, so just skip it
    return 1.f;
}

bool Infinite_sphere::thin_absorption(Ray const& /*ray*/, Transformation const& /*trafo*/,
                                      uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/,
                                      float3& ta) const {
    // Implementation for this is not really needed, so just skip it
    ta = float3(1.f);
    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& n,
                             Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                             Sampler& sampler, RNG& rng, uint32_t sampler_d,
                             Sample_to& sample) const {
    auto const [x, y] = orthonormal_basis(n);

    float2 const uv  = sampler.generate_sample_2D(rng, sampler_d);
    float3 const dir = sample_oriented_hemisphere_uniform(uv, x, y, n);

    float3 const xyz = normalize(transform_vector_transposed(trafo.rotation, dir));

    sample = Sample_to(dir,
                       float3(std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                              std::acos(xyz[1]) * Pi_inv, 0.f),
                       1.f / (2.f * Pi), Ray_max_t);

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, Transformation const& trafo,
                             float /*area*/, bool /*two_sided*/, Sampler& sampler, RNG& rng,
                             uint32_t sampler_d, Sample_to& sample) const {
    float2 const uv  = sampler.generate_sample_2D(rng, sampler_d);
    float3 const dir = sample_sphere_uniform(uv);

    float3 const xyz = normalize(transform_vector_transposed(trafo.rotation, dir));

    sample = Sample_to(dir,
                       float3(std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                              std::acos(xyz[1]) * Pi_inv, 0.f),
                       1.f / (2.f * Pi), Ray_max_t);

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, Transformation const& /*trafo*/, float /*area*/,
                             bool /*two_sided*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                             uint32_t /*sampler_d*/, float2 /*importance_uv*/,
                             AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Infinite_sphere::pdf(Ray const& /*ray*/, Intersection const& /*isec*/,
                           Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                           bool total_sphere) const {
    if (total_sphere) {
        return 1.f / (4.f * Pi);
    }

    return 1.f / (2.f * Pi);
}

float Infinite_sphere::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                                  Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float2 uv,
                             Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                             Sample_to&            sample) const {
    float const phi   = (uv[0] - 0.5f) * (2.f * Pi);
    float const theta = uv[1] * Pi;

    auto const [sin_phi, cos_phi] = sincos(phi);

    auto const [sin_theta, cos_theta] = sincos(theta);

    float3 const dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    sample = Sample_to(transform_vector(trafo.rotation, dir), float3(uv),
                       1.f / ((4.f * Pi) * sin_theta), Ray_max_t);

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                             Transformation const& /*trafo*/, float /*volume*/,
                             Sample_to& /*sample*/) const {
    return false;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float2 uv, Transformation const& trafo,
                             float /*area*/, bool /*two_sided*/, float2          importance_uv,
                             AABB const& bounds, Sample_from& sample) const {
    float const phi   = (uv[0] - 0.5f) * (2.f * Pi);
    float const theta = uv[1] * Pi;

    auto const [sin_phi, cos_phi] = sincos(phi);

    auto const [sin_theta, cos_theta] = sincos(theta);

    float3 const ls(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    float3 const ws = -transform_vector(trafo.rotation, ls);

    auto const [t, b] = orthonormal_basis(ws);

    float2 const r0 = importance_uv;  // sampler.generate_sample_2D(sampler_d);

    float const radius_2 = squared_length(bounds.halfsize());

    float const radius = std::sqrt(radius_2);

    float3x3 const rotation(t, b, ws);

    AABB const ls_bounds = bounds.transform(affine_inverted(float4x4(rotation)));

    float3 const origin = ls_bounds.min();
    float3 const pe     = float3(ls_bounds.max().xy(), origin[2]) - origin;

    float3 const receiver_rect = float3(r0 - 0.5f, 0.f) * pe;

    float3 const photon_rect = transform_vector(rotation, receiver_rect);

    float3 const pli = bounds.position() - radius * ws + photon_rect;

    float3 const disk = sample_oriented_disk_concentric(r0, t, b);

    float3 const p = bounds.position() + radius * (disk - ws);

    sample.dir = ws;
    sample.p   = pli;
    sample.uv  = uv;
    sample.xy  = importance_uv;
    // sin_theta because of the uv weight
    sample.pdf = 1.f / ((4.f * Pi) * (sin_theta * pe[0] * pe[1]));

    return true;
}

float Infinite_sphere::pdf_uv(Ray const& /*ray*/, Intersection const& isec,
                              Transformation const& /*trafo*/, float /*area*/,
                              bool /*two_sided*/) const {
    // sin_theta because of the uv weight
    float const sin_theta = std::sin(isec.uv[1] * Pi);

    if (0.f == sin_theta) {
        return 0.f;
    }

    return 1.f / ((4.f * Pi) * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const {
    float const sin_theta = std::sin(uv[1] * Pi);

    return sin_theta;
}

float Infinite_sphere::area(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 4.f * Pi;
}

float Infinite_sphere::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Infinite_sphere::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
