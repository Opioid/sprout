#include "canopy.hpp"
#include "base/math/aabb.inl"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

static float constexpr Canopy_eps = -0.0005f;

Canopy::Canopy() : Shape(Properties(Property::Analytical)) {}

AABB Canopy::aabb() const {
    return Empty_AABB;
}

bool Canopy::intersect(Ray& ray, Transformation const&      trafo, Worker& /*worker*/,
                       Interpolation /*ipo*/, Intersection& isec) const {
    if (ray.max_t() >= Ray_max_t) {
        if (dot(ray.direction, trafo.rotation.r[2]) < Canopy_eps) {
            return false;
        }

        isec.p = ray.point(Ray_max_t);
        isec.t = trafo.rotation.r[0];
        isec.b = trafo.rotation.r[1];

        float3 const n = -ray.direction;
        isec.n         = n;
        isec.geo_n     = n;
        isec.part      = 0;

        // paraboloid, so doesn't match hemispherical camera
        float3 xyz = transform_vector_transposed(trafo.rotation, ray.direction);
        xyz        = normalize(xyz);

        float2 const disk = hemisphere_to_disk_equidistant(xyz);
        isec.uv[0]        = 0.5f * disk[0] + 0.5f;
        isec.uv[1]        = 0.5f * disk[1] + 0.5f;

        ray.max_t() = Ray_max_t;

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Canopy::intersect_p(Ray const& /*ray*/, Transformation const& /*trafo*/,
                         Worker& /*worker*/) const {
    // Implementation for this is not really needed, so just skip it
    return false;
}

bool Canopy::visibility(Ray const& /*ray*/, Transformation const& /*trafo*/, uint32_t /*entity*/,
                        Filter /*filter*/, Worker& /*worker*/, float3& v) const {
    // Implementation for this is not really needed, so just skip it
    v = float3(1.f);
    return true;
}

bool Canopy::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p /*p*/, float3_p /*n*/,
                    Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                    bool /*total_sphere*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                    Sample_to& sample) const {
    float2 const uv  = sampler.sample_2D(rng, sampler_d);
    float3 const dir = sample_oriented_hemisphere_uniform(uv, trafo.rotation);

    float3 const xyz  = normalize(transform_vector_transposed(trafo.rotation, dir));
    float2 const disk = hemisphere_to_disk_equidistant(xyz);

    sample = Sample_to(dir, float3(0.f), float3(0.5f * disk[0] + 0.5f, 0.5f * disk[1] + 0.5f, 0.f),
                       1.f / (2.f * Pi), Ray_max_t);

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Canopy::sample(uint32_t /*part*/, uint32_t /*variant*/, Transformation const& /*trafo*/,
                    float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/, RNG& /*rng*/,
                    uint32_t /*sampler_d*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                    Sample_from& /*sample*/) const {
    return false;
}

float Canopy::pdf(uint32_t /*variant*/, Ray const& /*ray*/, float3_p /*n*/,
                  Intersection const& /*isec*/, Transformation const& /*trafo*/, float /*area*/,
                  bool /*two_sided*/, bool /*total_sphere*/) const {
    return 1.f / (2.f * Pi);
}

float Canopy::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                         Transformation const& /*trafo*/, float /*area*/) const {
    return 0.f;
}

bool Canopy::sample(uint32_t /*part*/, float3_p /*p*/, float2 uv, Transformation const& trafo,
                    float /*area*/, bool /*two_sided*/, Sample_to& sample) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        sample.pdf() = 0.f;
        return false;
    }

    float3 const dir = disk_to_hemisphere_equidistant(disk);

    sample = Sample_to(transform_vector(trafo.rotation, dir), float3(0.f), float3(uv),
                       1.f / (2.f * Pi), Ray_max_t);

    return true;
}

bool Canopy::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                           Transformation const& /*trafo*/, float /*volume*/,
                           Sample_to& /*sample*/) const {
    return false;
}

bool Canopy::sample(uint32_t /*part*/, float2 uv, Transformation const& trafo, float /*area*/,
                    bool /*two_sided*/, float2 importance_uv, AABB const& bounds,
                    Sample_from& sample) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        sample.pdf() = 0.f;
        return false;
    }

    float3 const ls = disk_to_hemisphere_equidistant(disk);

    float3 const ws = -transform_vector(trafo.rotation, ls);

    auto const [t, b] = orthonormal_basis(ws);

    float const bounds_radius_2 = squared_length(bounds.halfsize());

    float const bounds_radius = std::sqrt(bounds_radius_2);

    float3 const receciver_disk = sample_oriented_disk_concentric(importance_uv, t, b);

    float3 const p = bounds.position() + bounds_radius * (receciver_disk - ws);

    sample = Sample_from(p, float3(0.f), ws, uv, importance_uv,
                         1.f / ((2.f * Pi) * (1.f * Pi) * bounds_radius_2));

    return true;
}

float Canopy::pdf_uv(Ray const& /*ray*/, Intersection const& /*isec*/,
                     Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/) const {
    return 1.f / (2.f * Pi);
}

float Canopy::uv_weight(float2 uv) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        return 0.f;
    }

    return 1.f;
}

float Canopy::area(uint32_t /*part*/, float3_p /*scale*/) const {
    return 2.f * Pi;
}

float Canopy::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Canopy::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
