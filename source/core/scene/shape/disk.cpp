#include "disk.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Disk::Disk() : Shape(Properties(Property::Finite, Property::Analytical)) {}

float3 Disk::object_to_texture_point(float3 const& p) const {
    return (p - float3(-1.f)) * (1.f / float3(2.f, 2.f, 0.2f));
}

float3 Disk::object_to_texture_vector(float3 const& v) const {
    return v * (1.f / float3(2.f, 2.f, 0.2f));
}

AABB Disk::transformed_aabb(float4x4 const& m) const {
    return AABB(float3(-1.f, -1.f, -0.01f), float3(1.f, 1.f, 0.01f)).transform(m);
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*nodes*/,
                     Intersection& isec) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            isec.p     = p;
            isec.geo_n = normal;

            float3 const t = -transformation.rotation.r[0];
            float3 const b = -transformation.rotation.r[1];

            isec.t = t;
            isec.b = b;
            isec.n = normal;

            float3 const sk       = k / radius;
            float const  uv_scale = 0.5f * transformation.scale_z();
            isec.uv[0]            = (dot(t, sk) + 1.f) * uv_scale;
            isec.uv[1]            = (dot(b, sk) + 1.f) * uv_scale;

            isec.part = 0;

            SOFT_ASSERT(testing::check(isec, transformation, ray));

            ray.max_t() = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& /*nodes*/,
                         Intersection& isec) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            isec.p     = p;
            isec.geo_n = normal;

            float3 const t = -transformation.rotation.r[0];
            float3 const b = -transformation.rotation.r[1];

            float3 const sk       = k / radius;
            float const  uv_scale = 0.5f * transformation.scale_z();
            isec.uv[0]            = (dot(t, sk) + 1.f) * uv_scale;
            isec.uv[1]            = (dot(b, sk) + 1.f) * uv_scale;

            isec.part = 0;

            SOFT_ASSERT(testing::check(isec, transformation, ray));

            ray.max_t() = hit_t;
            return true;
        }
    }

    return false;
}

bool Disk::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*nodes*/,
                     Normals& normals) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            ray.max_t() = hit_t;

            normals.geo_n = normal;
            normals.n     = normal;

            return true;
        }
    }

    return false;
}

bool Disk::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& /*nodes*/) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            return true;
        }
    }

    return false;
}

float Disk::visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                       Filter filter, Worker& worker) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            float3 const sk       = k / radius;
            float const  uv_scale = 0.5f * transformation.scale_z();

            float2 const uv((-dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                            (-dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            return 1.f -
                   worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
        }
    }

    return 1.f;
}

bool Disk::thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                           Filter filter, Worker& worker, float3& ta) const {
    float3 const& normal = transformation.rotation.r[2];

    float const d     = dot(normal, transformation.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - transformation.position;
        float const  l = dot(k, k);

        float const radius = transformation.scale_x();

        if (l <= radius * radius) {
            float3 const sk       = k / radius;
            float const  uv_scale = 0.5f * transformation.scale_z();

            float2 const uv((dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
                            (dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

            ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, normal, uv,
                                                                          ray.time, filter, worker);

            return true;
        }
    }

    ta = float3(1.f);
    return true;
}

bool Disk::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                  float area, bool two_sided, Sampler& sampler, rnd::Generator& rng,
                  uint32_t sampler_dimension, Sample_to& sample) const {
    float2 const r2 = sampler.generate_sample_2D(rng, sampler_dimension);
    float2 const xy = sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transformation.scale_x() * transform_vector(transformation.rotation, ls);

    float3 const axis = ws - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    float3 const wi = axis / t;

    float c = -dot(transformation.rotation.r[2], wi);

    if (two_sided) {
        c = std::abs(c);
    }

    if (c <= Dot_min) {
        return false;
    }

    float const pdf = sl / (c * area);

    sample = Sample_to(wi, float3(0.f), pdf, offset_b(t));

    return true;
}

bool Disk::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                  bool /*two_sided*/, Sampler& sampler, rnd::Generator& rng,
                  uint32_t sampler_dimension, float2 importance_uv, AABB const& /*bounds*/,
                  Sample_from& sample) const {
    float2 const r0 = sampler.generate_sample_2D(rng, sampler_dimension);
    float2 const xy = sample_disk_concentric(r0);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transform_vector(transformation.rotation, transformation.scale_x() * ls);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, transformation.rotation);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / (Pi * area);

    return true;
}

float Disk::pdf(Ray const& ray, Intersection const& /*isec*/, Transformation const& transformation,
                float area, bool two_sided, bool /*total_sphere*/) const {
    float3 const normal = transformation.rotation.r[2];

    float c = -dot(normal, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t() * ray.max_t();
    return sl / (c * area);
}

float Disk::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                       Transformation const& /*transformation*/, float /*volume*/) const {
    return 0.f;
}

bool Disk::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Disk::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Disk::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Disk::pdf_uv(Ray const& /*ray*/, Intersection const& /*isec*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const {
    return 0.f;
}

float Disk::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Disk::area(uint32_t /*part*/, float3 const& scale) const {
    return Pi * scale[0] * scale[0];
}

float Disk::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Disk::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

float4 Disk::cone(uint32_t /*part*/) const {
    return float4(0.f, 0.f, 1.f, 1.f);
}

}  // namespace scene::shape
