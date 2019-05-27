#include "celestial_disk.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Celestial_disk::Celestial_disk() noexcept {}

float3 Celestial_disk::object_to_texture_point(float3 const& p) const noexcept {
    return p;
}

float3 Celestial_disk::object_to_texture_vector(float3 const& v) const noexcept {
    return v;
}

AABB Celestial_disk::transformed_aabb(float4x4 const& /*m*/, math::Transformation const& t) const
    noexcept {
    return transformed_aabb(t);
}

AABB Celestial_disk::transformed_aabb(math::Transformation const& /*t*/) const noexcept {
    return AABB::empty();
}

bool Celestial_disk::intersect(Ray& ray, Transformation const&           transformation,
                               Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t = hit_t;

        intersection.p     = ray.point(hit_t);
        intersection.geo_n = n;
        intersection.t     = transformation.rotation.r[0];
        intersection.b     = transformation.rotation.r[1];
        intersection.n     = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        intersection.uv[0] = (dot(intersection.t, sk) + 1.f) * 0.5f;
        intersection.uv[1] = (dot(intersection.b, sk) + 1.f) * 0.5f;

        intersection.part = 0;

        return true;
    }

    return false;
}

bool Celestial_disk::intersect_fast(Ray& ray, Transformation const&           transformation,
                                    Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t = hit_t;

        intersection.p     = ray.point(hit_t);
        intersection.geo_n = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        intersection.uv[0] = (dot(intersection.t, sk) + 1.f) * 0.5f;
        intersection.uv[1] = (dot(intersection.b, sk) + 1.f) * 0.5f;

        intersection.part = 0;

        return true;
    }

    return false;
}

bool Celestial_disk::intersect(Ray& ray, Transformation const&      transformation,
                               Node_stack& /*node_stack*/, Normals& normals) const noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        ray.max_t = Almost_ray_max_t;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Celestial_disk::intersect_p(Ray const& ray, Transformation const& transformation,
                                 Node_stack& /*node_stack*/) const noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        return true;
    }

    return false;
}

float Celestial_disk::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
                              Materials /*materials*/, Filter /*filter*/,
                              Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return 0.f;
}

bool Celestial_disk::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                                     Materials /*materials*/, Filter /*filter*/,
                                     Worker const& /*worker*/, float3& ta) const noexcept {
    // Implementation for this is not really needed, so just skip it
    ta = float3(1.f);
    return true;
}

bool Celestial_disk::sample(uint32_t /*part*/, float3 const& /*p*/,
                            Transformation const& transformation, float area, bool /*two_sided*/,
                            Sampler& sampler, uint32_t             sampler_dimension,
                            Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = transformation.scale[0];

    float3 const ws = radius * transform_vector(transformation.rotation, ls);

    sample.wi = normalize(ws - transformation.rotation.r[2]);

    sample.pdf = 1.f / area;
    sample.t   = Almost_ray_max_t;

    return true;
}

bool Celestial_disk::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                            bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                            float2 const& importance_uv, AABB const& bounds,
                            Node_stack& /*node_stack*/, Sample_from& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = transformation.scale[0];

    float3 const ws = radius * transform_vector(transformation.rotation, ls);

    float3 const dir = normalize(transformation.rotation.r[2] - ws);

    AABB const ls_bounds = bounds.transform_transposed(transformation.rotation);

    float3 const ls_extent = ls_bounds.max() - ls_bounds.min();

    float2 const ls_rect = float2(ls_extent[0], ls_extent[1]);

    float3 const photon_rect = transform_vector(transformation.rotation,
                                                float3((importance_uv - 0.5f) * ls_rect, 0.f));

    float const bounds_radius = 0.5f * ls_extent[2];

    float3 const offset = bounds_radius * dir;

    float3 const p = ls_bounds.position() - offset + photon_rect;

    sample.dir = dir;
    sample.p   = p;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / (area * ls_rect[0] * ls_rect[1]);

    return true;
}

float Celestial_disk::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                          Transformation const& /*transformation*/, float area, bool /*two_sided*/,
                          bool /*total_sphere*/) const noexcept {
    return 1.f / area;
}

float Celestial_disk::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                                 Transformation const& /*transformation*/, float /*volume*/) const
    noexcept {
    return 0.f;
}

bool Celestial_disk::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Celestial_disk::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                            Transformation const& /*transformation*/, float /*volume*/,
                            Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Celestial_disk::sample(uint32_t /*part*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, float2 const& /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Celestial_disk::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                             Transformation const& /*transformation*/, float area,
                             bool /*two_sided*/) const noexcept {
    return 1.f / area;
}

float Celestial_disk::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Celestial_disk::area(uint32_t /*part*/, float3 const& scale) const noexcept {
    float const radius = scale[0];
    return Pi * (radius * radius);
}

float Celestial_disk::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

bool Celestial_disk::is_finite() const noexcept {
    return false;
}

size_t Celestial_disk::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
