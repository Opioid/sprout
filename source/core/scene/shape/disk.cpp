#include "disk.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Disk::Disk() {
    aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                     Intersection& intersection) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

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
            intersection.uv[0] = (math::dot(t, sk) + 1.f) * uv_scale;
            intersection.uv[1] = (math::dot(b, sk) + 1.f) * uv_scale;

            intersection.epsilon = 5e-4f * hit_t;
            intersection.part    = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect_fast(Ray& ray, Transformation const&           transformation,
                          Node_stack& /*node_stack*/, Intersection& intersection) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            intersection.p     = p;
            intersection.geo_n = normal;

            float3 t = -transformation.rotation.r[0];
            float3 b = -transformation.rotation.r[1];

            float3 sk          = k / radius;
            float  uv_scale    = 0.5f * transformation.scale[2];
            intersection.uv[0] = (math::dot(t, sk) + 1.f) * uv_scale;
            intersection.uv[1] = (math::dot(b, sk) + 1.f) * uv_scale;

            intersection.epsilon = 5e-4f * hit_t;
            intersection.part    = 0;

            SOFT_ASSERT(testing::check(intersection, transformation, ray));

            ray.max_t = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                     float& epsilon) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            epsilon = 5e-4f * hit_t;

            ray.max_t = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& /*node_stack*/) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            return true;
        }
    }

    return false;
}

float Disk::opacity(Ray const& ray, Transformation const& transformation,
                    Materials const& materials, Sampler_filter filter, Worker const& worker) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            float3 sk       = k / radius;
            float  uv_scale = 0.5f * transformation.scale[2];
            float2 uv((-math::dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                      (-math::dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            return materials[0]->opacity(uv, ray.time, filter, worker);
        }
    }

    return 0.f;
}

float3 Disk::thin_absorption(Ray const& ray, Transformation const& transformation,
                             Materials const& materials, Sampler_filter filter,
                             Worker const& worker) const {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = math::dot(normal, transformation.position);
    float         denom  = -math::dot(normal, ray.direction);
    float         numer  = math::dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;
        float  l = math::dot(k, k);

        float radius = transformation.scale[0];

        if (l <= radius * radius) {
            float3 sk       = k / radius;
            float  uv_scale = 0.5f * transformation.scale[2];
            float2 uv((math::dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                      (math::dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            return materials[0]->thin_absorption(ray.direction, normal, uv, ray.time, filter,
                                                 worker);
        }
    }

    return float3(0.f);
}

bool Disk::sample(uint32_t part, f_float3 p, f_float3 /*n*/, Transformation const& transformation,
                  float area, bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                  Node_stack& node_stack, Sample_to& sample) const {
    return Disk::sample(part, p, transformation, area, two_sided, sampler, sampler_dimension,
                        node_stack, sample);
}

bool Disk::sample(uint32_t /*part*/, f_float3 p, Transformation const& transformation, float area,
                  bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                  Node_stack& /*node_stack*/, Sample_to& sample) const {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = math::sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transformation.scale[0] * math::transform_vector(ls, transformation.rotation);

    float3 const axis = ws - p;

    float const sl = math::squared_length(axis);
    float const t  = std::sqrt(sl);

    float3 const wi = axis / t;

    float c = -math::dot(transformation.rotation.r[2], wi);

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

    sample.wi      = wi;
    sample.pdf     = pdf;
    sample.t       = t;
    sample.epsilon = 5e-4f * t;

    return true;
}

bool Disk::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                  bool /*two_sided*/, sampler::Sampler& sampler, uint32_t sampler_dimension,
                  Node_stack& /*node_stack*/, Sample_from& sample) const {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = math::sample_disk_concentric(r0);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      math::transform_vector(transformation.scale[0] * ls, transformation.rotation);

    float2 const r1  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_hemisphere_cosine(r1, transformation.rotation);

    sample.p       = ws;
    sample.dir     = dir;
    sample.pdf     = 1.f / (math::Pi * area);
    sample.epsilon = 5e-4f;

    return true;
}

float Disk::pdf(Ray const&            ray, const shape::Intersection& /*intersection*/,
                Transformation const& transformation, float area, bool two_sided,
                bool /*total_sphere*/) const {
    float3 const normal = transformation.rotation.r[2];

    float c = -math::dot(normal, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float sl = ray.max_t * ray.max_t;
    return sl / (c * area);
}

bool Disk::sample(uint32_t /*part*/, f_float3 /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Disk::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                  uint32_t /*sampler_dimension*/, Sample_from& /*sample*/) const {
    return false;
}

float Disk::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const {
    return 0.f;
}

float Disk::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Disk::area(uint32_t /*part*/, f_float3 scale) const {
    return math::Pi * scale[0] * scale[0];
}

size_t Disk::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::shape
