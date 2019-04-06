#include "disk.hpp"
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

Disk::Disk() noexcept {
    aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                     Intersection& intersection) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            intersection.p     = p;
            intersection.geo_n = normal;
            float3 t           = -transformation.rotation.r[0];
            float3 b           = -transformation.rotation.r[1];
            intersection.t     = t;
            intersection.b     = b;
            intersection.n     = normal;

            float3 sk          = k / radius;
            float  uv_scale    = 0.5f * transformation.scale[2];
            intersection.uv[0] = (dot(t, sk) + 1.f) * uv_scale;
            intersection.uv[1] = (dot(b, sk) + 1.f) * uv_scale;

            intersection.part = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect_fast(Ray& ray, Transformation const&           transformation,
                          Node_stack& /*node_stack*/, Intersection& intersection) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            intersection.p     = p;
            intersection.geo_n = normal;

            float3 t = -transformation.rotation.r[0];
            float3 b = -transformation.rotation.r[1];

            float3 sk          = k / radius;
            float  uv_scale    = 0.5f * transformation.scale[2];
            intersection.uv[0] = (dot(t, sk) + 1.f) * uv_scale;
            intersection.uv[1] = (dot(b, sk) + 1.f) * uv_scale;

            intersection.part = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                     Normals& normals) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            ray.max_t = hit_t;

            normals.geo_n = normal;
            normals.n     = normal;

            return true;
        }
    }

    return false;
}

bool Disk::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& /*node_stack*/) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            return true;
        }
    }

    return false;
}

float Disk::opacity(Ray const& ray, Transformation const& transformation, Materials materials,
                    Filter filter, Worker const& worker) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            float3 sk       = k / radius;
            float  uv_scale = 0.5f * transformation.scale[2];
            float2 uv((-dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                      (-dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            return materials[0]->opacity(uv, ray.time, filter, worker);
        }
    }

    return 0.f;
}

bool Disk::thin_absorption(Ray const& ray, Transformation const& transformation,
                           Materials materials, Filter filter, Worker const& worker,
                           float3& ta) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            float3 sk       = k / radius;
            float  uv_scale = 0.5f * transformation.scale[2];
            float2 uv((dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                      (dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            ta = materials[0]->thin_absorption(ray.direction, normal, uv, ray.time, filter, worker);

            return true;
        }
    }

    ta = float3(1.f);
    return true;
}

bool Disk::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                  float area, bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                  Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transformation.scale[0] * transform_vector(transformation.rotation, ls);

    float3 const axis = ws - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    float3 const wi = axis / t;

    float c = -dot(transformation.rotation.r[2], wi);

    if (two_sided) {
        c = std::abs(c);
    }

    if (c <= 0.f) {
        return false;
    }

    float const pdf = sl / (c * area);

    if (pdf > 1.6e19f) {
        return false;
    }

    sample.wi  = wi;
    sample.pdf = pdf;
    sample.t   = offset_b(t);

    return true;
}

bool Disk::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                  bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                  float2 const& importance_uv, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                  Sample_from&  sample) const noexcept {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = sample_disk_concentric(r0);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transform_vector(transformation.rotation, transformation.scale[0] * ls);

    float3 const dir = math::sample_oriented_hemisphere_cosine(importance_uv,
                                                               transformation.rotation);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / (Pi * area);

    return true;
}

float Disk::pdf(Ray const&            ray, Intersection const& /*intersection*/,
                Transformation const& transformation, float area, bool two_sided,
                bool /*total_sphere*/) const noexcept {
    float3 const normal = transformation.rotation.r[2];

    float c = -dot(normal, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float sl = ray.max_t * ray.max_t;
    return sl / (c * area);
}

float Disk::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float /*volume*/) const noexcept {
    return 0.f;
}

bool Disk::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Disk::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Disk::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                  uint32_t /*sampler_dimension*/, float2 const& /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Disk::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const noexcept {
    return 0.f;
}

float Disk::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Disk::area(uint32_t /*part*/, float3 const& scale) const noexcept {
    return Pi * scale[0] * scale[0];
}

float Disk::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

size_t Disk::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
