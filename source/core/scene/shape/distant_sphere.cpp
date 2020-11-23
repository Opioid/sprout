#include "distant_sphere.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Distant_sphere::Distant_sphere() : Shape(Properties(Property::Analytical)) {}

float3 Distant_sphere::object_to_texture_point(float3_p p) const {
    return p;
}

float3 Distant_sphere::object_to_texture_vector(float3_p v) const {
    return v;
}

AABB Distant_sphere::transformed_aabb(float4x4 const& /*m*/) const {
    return AABB::empty();
}

bool Distant_sphere::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                               Intersection& isec) const {
    float3 const n = trafo.rotation.r[2];

    float const b = dot(n, ray.direction);

    if ((b > 0.f) | (ray.max_t() < Ray_max_t)) {
        return false;
    }

    float const radius = trafo.scale_x();
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t() = hit_t;

        isec.p     = ray.point(hit_t);
        isec.geo_n = n;
        isec.t     = trafo.rotation.r[0];
        isec.b     = trafo.rotation.r[1];
        isec.n     = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        isec.uv[0] = (dot(isec.t, sk) + 1.f) * 0.5f;
        isec.uv[1] = (dot(isec.b, sk) + 1.f) * 0.5f;

        isec.part = 0;

        return true;
    }

    return false;
}

bool Distant_sphere::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                                   Intersection& isec) const {
    float3 const n = trafo.rotation.r[2];

    float const b = dot(n, ray.direction);

    if ((b > 0.f) | (ray.max_t() < Ray_max_t)) {
        return false;
    }

    float const radius = trafo.scale_x();
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t() = hit_t;

        isec.p     = ray.point(hit_t);
        isec.geo_n = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        isec.uv[0] = (dot(isec.t, sk) + 1.f) * 0.5f;
        isec.uv[1] = (dot(isec.b, sk) + 1.f) * 0.5f;

        isec.part = 0;

        return true;
    }

    return false;
}

bool Distant_sphere::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                               Normals& normals) const {
    float3 const n = trafo.rotation.r[2];

    float const b = dot(n, ray.direction);

    if ((b > 0.f) | (ray.max_t() < Ray_max_t)) {
        return false;
    }

    float const radius = trafo.scale_x();
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        ray.max_t() = Almost_ray_max_t;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Distant_sphere::intersect_p(Ray const& ray, Transformation const& trafo,
                                 Node_stack& /*nodes*/) const {
    float3 const n = trafo.rotation.r[2];

    float const b = dot(n, ray.direction);

    if ((b > 0.f) | (ray.max_t() < Ray_max_t)) {
        return false;
    }

    float const radius = trafo.scale_x();
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    return det > 0.f;
}

float Distant_sphere::visibility(Ray const& /*ray*/, Transformation const& /*trafo*/,
                                 uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/) const {
    // Implementation for this is not really needed, so just skip it
    return 1.f;
}

bool Distant_sphere::thin_absorption(Ray const& /*ray*/, Transformation const& /*trafo*/,
                                     uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/,
                                     float3& ta) const {
    // Implementation for this is not really needed, so just skip it
    ta = float3(1.f);
    return true;
}

bool Distant_sphere::sample(uint32_t /*part*/, float3_p /*p*/, Transformation const& trafo,
                            float area, bool /*two_sided*/, Sampler& sampler, RNG& rng,
                            uint32_t sampler_d, Sample_to& sample) const {
    float2 const r2 = sampler.sample_2D(rng, sampler_d);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = trafo.scale_x();

    float3 const ws = radius * transform_vector(trafo.rotation, ls);

    sample = Sample_to(normalize(ws - trafo.rotation.r[2]), float3(0.f), 1.f / area,
                       Almost_ray_max_t);

    return true;
}

bool Distant_sphere::sample(uint32_t /*part*/, Transformation const& trafo, float area,
                            bool /*two_sided*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                            float2 importance_uv, AABB const& bounds, Sample_from& sample) const {
    float2 const r2 = sampler.sample_2D(rng, sampler_d);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = trafo.scale_x();

    float3 const ws = radius * transform_vector(trafo.rotation, ls);

    float3 const dir = normalize(trafo.rotation.r[2] - ws);

    AABB const ls_bounds = bounds.transform_transposed(trafo.rotation);

    float3 const ls_extent = ls_bounds.max() - ls_bounds.min();

    float2 const ls_rect = float2(ls_extent[0], ls_extent[1]);

    float3 const photon_rect = transform_vector(trafo.rotation,
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

float Distant_sphere::pdf(Ray const& /*ray*/, float3_p /*n*/, Intersection const& /*isec*/,
                          Transformation const& /*trafo*/, float area, bool /*two_sided*/,
                          bool /*total_sphere*/) const {
    return 1.f / area;
}

float Distant_sphere::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                                 Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Distant_sphere::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/,
                            Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                            Sample_to& /*sample*/) const {
    return false;
}

bool Distant_sphere::sample(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                            Transformation const& /*trafo*/, float /*volume*/,
                            Sample_to& /*sample*/) const {
    return false;
}

bool Distant_sphere::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/,
                            float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Distant_sphere::pdf_uv(Ray const& /*ray*/, Intersection const& /*isec*/,
                             Transformation const& /*trafo*/, float area,
                             bool /*two_sided*/) const {
    return 1.f / area;
}

float Distant_sphere::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Distant_sphere::area(uint32_t /*part*/, float3_p scale) const {
    float const radius = scale[0];
    return Pi * (radius * radius);
}

float Distant_sphere::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Distant_sphere::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

float4 Distant_sphere::cone(uint32_t /*part*/) const {
    return float4(0.f, 0.f, 1.f, 0.f);
}

}  // namespace scene::shape
