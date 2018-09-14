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

Infinite_sphere::Infinite_sphere() noexcept {
    aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const&           transformation,
                                Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    if (ray.max_t >= Ray_max_t) {
        intersection.epsilon = 5e-4f;

        // This is nonsense
        intersection.t = transformation.rotation.r[0];
        intersection.b = transformation.rotation.r[1];

        float3 xyz = math::transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = math::normalize(xyz);
        intersection.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
        intersection.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

        intersection.p     = ray.point(Ray_max_t);
        float3 const n     = -ray.direction;
        intersection.n     = n;
        intersection.geo_n = n;
        intersection.part  = 0;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect_fast(Ray& ray, Transformation const&           transformation,
                                     Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    if (ray.max_t >= Ray_max_t) {
        intersection.epsilon = 5e-4f;

        float3 xyz = math::transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = math::normalize(xyz);
        intersection.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
        intersection.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

        intersection.p     = ray.point(Ray_max_t);
        float3 const n     = -ray.direction;
        intersection.geo_n = n;
        intersection.part  = 0;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Infinite_sphere::intersect(Ray& ray, Transformation const& /*transformation*/,
                                Node_stack& /*node_stack*/, float& epsilon) const noexcept {
    if (ray.max_t >= Ray_max_t) {
        ray.max_t = Ray_max_t;
        epsilon   = 5e-4f;
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
                               Materials const& /*materials*/, Sampler_filter /*filter*/,
                               Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return 0.f;
}

float3 Infinite_sphere::thin_absorption(Ray const& /*ray*/,
                                        Transformation const& /*transformation*/,
                                        Materials const& /*materials*/, Sampler_filter /*filter*/,
                                        Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return float3(0.f);
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& n,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, sampler::Sampler& sampler,
                             uint32_t   sampler_dimension, Node_stack& /*node_stack*/,
                             Sample_to& sample) const noexcept {
    float3 x, y;
    math::orthonormal_basis(n, x, y);

    float2 const uv  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

    sample.wi = dir;

    float3 const xyz = math::normalize(
        math::transform_vector_transposed(transformation.rotation, dir));

    sample.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
    sample.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

    sample.pdf = 1.f / (2.f * math::Pi);

    sample.t       = Ray_max_t;
    sample.epsilon = 5e-4f;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, sampler::Sampler& sampler,
                             uint32_t   sampler_dimension, Node_stack& /*node_stack*/,
                             Sample_to& sample) const noexcept {
    float2 const uv  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_sphere_uniform(uv);

    sample.wi = dir;

    float3 const xyz = math::normalize(
        math::transform_vector_transposed(transformation.rotation, dir));

    sample.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
    sample.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

    sample.pdf = 1.f / (4.f * math::Pi);

    sample.t       = Ray_max_t;
    sample.epsilon = 5e-4f;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, Transformation const& /*transformation*/,
                             float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                             uint32_t /*sampler_dimension*/, math::AABB const& /*bounds*/,
                             Node_stack& /*node_stack*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Infinite_sphere::pdf(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
                           Transformation const& /*transformation*/, float /*area*/,
                           bool /*two_sided*/, bool total_sphere) const noexcept {
    if (total_sphere) {
        return 1.f / (4.f * math::Pi);
    } else {
        return 1.f / (2.f * math::Pi);
    }
}

bool Infinite_sphere::sample(uint32_t /*part*/, float3 const& /*p*/, float2 uv,
                             Transformation const& transformation, float /*area*/,
                             bool /*two_sided*/, Sample_to& sample) const noexcept {
    float const phi   = (uv[0] - 0.5f) * (2.f * math::Pi);
    float const theta = uv[1] * math::Pi;

    float sin_phi;
    float cos_phi;
    math::sincos(phi, sin_phi, cos_phi);

    float sin_theta;
    float cos_theta;
    math::sincos(theta, sin_theta, cos_theta);

    float3 const dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    sample.wi = math::transform_vector(transformation.rotation, dir);
    sample.uv = uv;
    // sin_theta because of the uv weight
    sample.pdf = 1.f / ((4.f * math::Pi) * sin_theta);

    sample.t       = Ray_max_t;
    sample.epsilon = 5e-4f;

    SOFT_ASSERT(testing::check(sample, uv));

    return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, float2 uv, Transformation const&   transformation,
                             float /*area*/, bool /*two_sided*/, sampler::Sampler& sampler,
                             uint32_t sampler_dimension, math::AABB const& bounds,
                             Sample_from& sample) const noexcept {
    float const phi   = (uv[0] - 0.5f) * (2.f * math::Pi);
    float const theta = uv[1] * math::Pi;

    float sin_phi;
    float cos_phi;
    math::sincos(phi, sin_phi, cos_phi);

    float sin_theta;
    float cos_theta;
    math::sincos(theta, sin_theta, cos_theta);

    float3 const ls(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    float3 const ws = -math::transform_vector(transformation.rotation, ls);

    float3 t, b;
    math::orthonormal_basis(ws, t, b);

    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);

    float const radius = math::length(bounds.halfsize());

    float3 const disk = math::sample_oriented_disk_concentric(r0, t, b);

    float3 const p = bounds.position() + radius * (disk - ws);

    sample.dir = ws;
    sample.p   = p;
    sample.uv  = uv;
    // sin_theta because of the uv weight
    sample.pdf     = 1.f / ((4.f * math::Pi) * (1.f * math::Pi) * (sin_theta * radius * radius));
    sample.epsilon = 5e-4f;

    return true;
}

float Infinite_sphere::pdf_uv(Ray const& /*ray*/, Intersection const& intersection,
                              Transformation const& /*transformation*/, float /*area*/,
                              bool /*two_sided*/) const noexcept {
    //	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
    //	xyz = math::normalize(xyz);
    //	uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
    //	uv[1] = std::acos(xyz[1]) * math::Pi_inv;

    //	// sin_theta because of the uv weight
    //	float const sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(intersection.uv[1] * math::Pi);

    return 1.f / ((4.f * math::Pi) * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const noexcept {
    float const sin_theta = std::sin(uv[1] * math::Pi);

    return sin_theta;
}

float Infinite_sphere::area(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 4.f * math::Pi;
}

bool Infinite_sphere::is_finite() const noexcept {
    return false;
}

size_t Infinite_sphere::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
