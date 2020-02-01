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

Infinite_sphere::Infinite_sphere() noexcept = default;

float3 Infinite_sphere::object_to_texture_point(float3 const& p) const noexcept {
    return p;
}

float3 Infinite_sphere::object_to_texture_vector(float3 const& v) const noexcept {
    return v;
}

AABB Infinite_sphere::transformed_aabb(float4x4 const& /*m*/) const noexcept {
    return AABB::empty();
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const&           transformation,
                                Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    if (ray.max_t >= Ray_max_t) {
        // This is nonsense
        intersection.t = transformation.rotation.r[0];
        intersection.b = transformation.rotation.r[1];

        float3 xyz = transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = normalize(xyz);

        intersection.uv[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
        intersection.uv[1] = std::acos(xyz[1]) * Pi_inv;

        intersection.p = ray.point(Ray_max_t);

        float3 const n = -ray.direction;

        intersection.n     = n;
        intersection.geo_n = n;
        intersection.part  = 0;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect_nsf(Ray& ray, Transformation const&           transformation,
                                    Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    if (ray.max_t >= Ray_max_t) {
        float3 xyz = transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = normalize(xyz);

        intersection.uv[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
        intersection.uv[1] = std::acos(xyz[1]) * Pi_inv;

        intersection.p = ray.point(Ray_max_t);

        float3 const n = -ray.direction;

        intersection.geo_n = n;
        intersection.part  = 0;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const& /*transformation*/,
                                Node_stack& /*node_stack*/, Normals& normals) const noexcept {
    if (ray.max_t >= Ray_max_t) {
        ray.max_t = Ray_max_t;

        float3 const n = -ray.direction;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect_p(Ray const& /*ray*/, Transformation const& /*transformation*/,
                                  Node_stack& /*node_stack*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return false;
}

float Infinite_sphere::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
                               uint32_t /*entity*/, Filter /*filter*/,
                               Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return 0.f;
}

bool Infinite_sphere::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                                      uint32_t /*entity*/, Filter /*filter*/,
                                      Worker const& /*worker*/, float3& ta) const noexcept {
    // Implementation for this is not really needed, so just skip it
    ta = float3(1.f);
    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& n,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                             Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    auto const [x, y] = orthonormal_basis(n);

    float2 const uv  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

    sample.wi = dir;

    float3 const xyz = normalize(transform_vector_transposed(transformation.rotation, dir));

    sample.uvw[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    sample.uvw[1] = std::acos(xyz[1]) * Pi_inv;

    sample.pdf = 1.f / (2.f * Pi);

    sample.t = Ray_max_t;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                             Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float2 const uv  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_sphere_uniform(uv);

    sample.wi = dir;

    float3 const xyz = normalize(transform_vector_transposed(transformation.rotation, dir));

    sample.uvw[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    sample.uvw[1] = std::acos(xyz[1]) * Pi_inv;

    sample.pdf = 1.f / (4.f * Pi);

    sample.t = Ray_max_t;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, Transformation const& /*transformation*/,
                             float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                             uint32_t /*sampler_dimension*/, float2 /*importance_uv*/,
                             AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                             Sample_from& /*sample*/) const noexcept {
    return false;
}

float Infinite_sphere::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                           Transformation const& /*transformation*/, float /*area*/,
                           bool /*two_sided*/, bool total_sphere) const noexcept {
    if (total_sphere) {
        return 1.f / (4.f * Pi);
    }

    return 1.f / (2.f * Pi);
}

float Infinite_sphere::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                                  Transformation const& /*transformation*/, float /*volume*/) const
    noexcept {
    return 0.f;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float2 uv,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, Sample_to& sample) const noexcept {
    float const phi   = (uv[0] - 0.5f) * (2.f * Pi);
    float const theta = uv[1] * Pi;

    auto const [sin_phi, cos_phi] = sincos(phi);

    auto const [sin_theta, cos_theta] = sincos(theta);

    float3 const dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    sample.wi  = transform_vector(transformation.rotation, dir);
    sample.uvw = float3(uv);
    // sin_theta because of the uv weight
    sample.pdf = 1.f / ((4.f * Pi) * sin_theta);

    sample.t = Ray_max_t;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                             Transformation const& /*transformation*/, float /*volume*/,
                             Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float2 uv, Transformation const& transformation,
                             float /*area*/, bool /*two_sided*/, float2          importance_uv,
                             AABB const& bounds, Sample_from& sample) const noexcept {
    float const phi   = (uv[0] - 0.5f) * (2.f * Pi);
    float const theta = uv[1] * Pi;

    auto const [sin_phi, cos_phi] = sincos(phi);

    auto const [sin_theta, cos_theta] = sincos(theta);

    float3 const ls(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    float3 const ws = -transform_vector(transformation.rotation, ls);

    auto const [t, b] = orthonormal_basis(ws);

    float2 const r0 = importance_uv;  // sampler.generate_sample_2D(sampler_dimension);

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
    //  sample.pdf     = 1.f / ((4.f * Pi) * (1.f * Pi) * (sin_theta * radius_2));
    sample.pdf = 1.f / ((4.f * Pi) * (sin_theta * pe[0] * pe[1]));

    return true;
}

float Infinite_sphere::pdf_uv(Ray const& /*ray*/, Intersection const& intersection,
                              Transformation const& /*transformation*/, float /*area*/,
                              bool /*two_sided*/) const noexcept {
    //	float3 xyz = transform_vector_transposed(wi, transformation.rotation);
    //	xyz = normalize(xyz);
    //	uv[0] = std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    //	uv[1] = std::acos(xyz[1]) * Pi_inv;

    //	// sin_theta because of the uv weight
    //	float const sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(intersection.uv[1] * Pi);

    return 1.f / ((4.f * Pi) * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const noexcept {
    float const sin_theta = std::sin(uv[1] * Pi);

    return sin_theta;
}

float Infinite_sphere::area(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 4.f * Pi;
}

float Infinite_sphere::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

bool Infinite_sphere::is_finite() const noexcept {
    return false;
}

size_t Infinite_sphere::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
