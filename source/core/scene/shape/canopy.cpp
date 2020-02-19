#include "canopy.hpp"
#include "base/math/aabb.inl"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
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

static float constexpr Canopy_eps = -0.0005f;

Canopy::Canopy() = default;

float3 Canopy::object_to_texture_point(float3 const& p) const {
    return p;
}

float3 Canopy::object_to_texture_vector(float3 const& v) const {
    return v;
}

AABB Canopy::transformed_aabb(float4x4 const& /*m*/) const {
    return AABB::empty();
}

bool Canopy::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       Intersection& intersection) const {
    if (ray.max_t >= Ray_max_t) {
        if (dot(ray.direction, transformation.rotation.r[2]) < Canopy_eps) {
            return false;
        }

        intersection.p = ray.point(Ray_max_t);
        intersection.t = transformation.rotation.r[0];
        intersection.b = transformation.rotation.r[1];

        float3 const n     = -ray.direction;
        intersection.n     = n;
        intersection.geo_n = n;
        intersection.part  = 0;

        // paraboloid, so doesn't match hemispherical camera
        float3 xyz = transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = normalize(xyz);

        float2 const disk  = hemisphere_to_disk_equidistant(xyz);
        intersection.uv[0] = 0.5f * disk[0] + 0.5f;
        intersection.uv[1] = 0.5f * disk[1] + 0.5f;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Canopy::intersect_nsf(Ray& ray, Transformation const&           transformation,
                           Node_stack& /*node_stack*/, Intersection& intersection) const {
    if (ray.max_t >= Ray_max_t) {
        if (dot(ray.direction, transformation.rotation.r[2]) < Canopy_eps) {
            return false;
        }

        intersection.p = ray.point(Ray_max_t);

        float3 const n = -ray.direction;

        intersection.geo_n = n;
        intersection.part  = 0;

        // paraboloid, so doesn't match hemispherical camera
        float3 xyz = transform_vector_transposed(transformation.rotation, ray.direction);
        xyz        = normalize(xyz);

        float2 const disk  = hemisphere_to_disk_equidistant(xyz);
        intersection.uv[0] = 0.5f * disk[0] + 0.5f;
        intersection.uv[1] = 0.5f * disk[1] + 0.5f;

        ray.max_t = Ray_max_t;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Canopy::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                       Normals& normals) const {
    if (ray.max_t >= Ray_max_t) {
        if (dot(ray.direction, transformation.rotation.r[2]) < Canopy_eps) {
            return false;
        }

        ray.max_t = Ray_max_t;

        float3 const n = -ray.direction;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Canopy::intersect_p(Ray const& /*ray*/, Transformation const& /*transformation*/,
                         Node_stack& /*node_stack*/) const {
    // Implementation for this is not really needed, so just skip it
    return false;
}

float Canopy::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
                      uint32_t /*entity*/, Filter /*filter*/, Worker const& /*worker*/) const {
    // Implementation for this is not really needed, so just skip it
    return 0.f;
}

bool Canopy::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                             uint32_t /*entity*/, Filter /*filter*/, Worker const& /*worker*/,
                             float3& ta) const {
    // Implementation for this is not really needed, so just skip it
    ta = float3(1.f);
    return true;
}

bool Canopy::sample(uint32_t /*part*/, float3 const& /*p*/, Transformation const& transformation,
                    float /*area*/, bool /*two_sided*/, Sampler&                  sampler,
                    uint32_t sampler_dimension, Sample_to& sample) const {
    float2 const uv  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = sample_oriented_hemisphere_uniform(uv, transformation.rotation);

    float3 const xyz  = normalize(transform_vector_transposed(transformation.rotation, dir));
    float2 const disk = hemisphere_to_disk_equidistant(xyz);

    sample.wi     = dir;
    sample.uvw[0] = 0.5f * disk[0] + 0.5f;
    sample.uvw[1] = 0.5f * disk[1] + 0.5f;
    sample.pdf    = 1.f / (2.f * Pi);
    sample.t      = Ray_max_t;

    SOFT_ASSERT(testing::check(sample));

    return true;
}

bool Canopy::sample(uint32_t /*part*/, Transformation const& /*transformation*/, float /*area*/,
                    bool /*two_sided*/, Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                    float2 /*importance_uv*/, AABB const& /*bounds*/,
                    Sample_from& /*sample*/) const {
    return false;
}

float Canopy::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/) const {
    return 1.f / (2.f * Pi);
}

float Canopy::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                         Transformation const& /*transformation*/, float /*area*/) const {
    return 0.f;
}

bool Canopy::sample(uint32_t /*part*/, float3 const& /*p*/, float2 uv,
                    Transformation const& transformation, float /*area*/, bool /*two_sided*/,
                    Sample_to&            sample) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        sample.pdf = 0.f;
        return false;
    }

    float3 const dir = disk_to_hemisphere_equidistant(disk);

    sample.wi  = transform_vector(transformation.rotation, dir);
    sample.uvw = float3(uv);
    sample.t   = Ray_max_t;
    sample.pdf = 1.f / (2.f * Pi);

    return true;
}

bool Canopy::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                    Transformation const& /*transformation*/, float /*volume*/,
                    Sample_to& /*sample*/) const {
    return false;
}

bool Canopy::sample(uint32_t /*part*/, float2 uv, Transformation const& transformation,
                    float /*area*/, bool /*two_sided*/, float2 importance_uv, AABB const& bounds,
                    Sample_from& sample) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        sample.pdf = 0.f;
        return false;
    }

    float3 const ls = disk_to_hemisphere_equidistant(disk);

    float3 const ws = -transform_vector(transformation.rotation, ls);

    auto const [t, b] = orthonormal_basis(ws);

    float const bounds_radius_2 = squared_length(bounds.halfsize());

    float const bounds_radius = std::sqrt(bounds_radius_2);

    float3 const receciver_disk = sample_oriented_disk_concentric(importance_uv, t, b);

    float3 const p = bounds.position() + bounds_radius * (receciver_disk - ws);

    sample.dir = ws;
    sample.p   = p;
    sample.uv  = uv;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / ((2.f * Pi) * (1.f * Pi) * bounds_radius_2);

    return true;
}

float Canopy::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                     Transformation const& /*transformation*/, float /*area*/,
                     bool /*two_sided*/) const {
    return 1.f / (2.f * Pi);
}

float Canopy::uv_weight(float2 uv) const {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    if (float const z = dot(disk, disk); z > 1.f) {
        return 0.f;
    }

    return 1.f;
}

float Canopy::area(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 2.f * Pi;
}

float Canopy::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

bool Canopy::is_finite() const {
    return false;
}

size_t Canopy::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::shape
