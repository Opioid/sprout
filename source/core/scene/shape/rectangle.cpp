#include "rectangle.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/material.hpp"
#include "scene/material/material.inl"
#include "scene/ray_offset.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Rectangle::Rectangle() : Shape(Properties(Property::Finite, Property::Analytical)) {}

AABB Rectangle::aabb() const {
    return AABB(float3(-1.f, -1.f, -0.01f), float3(1.f, 1.f, 0.01f));
}

bool Rectangle::intersect(Ray& ray, Transformation const&      trafo, Node_stack& /*nodes*/,
                          Interpolation /*ipo*/, Intersection& isec) const {
    float3_p normal = trafo.rotation.r[2];

    float const d     = dot(normal, trafo.position);
    float const denom = -dot(normal, ray.direction);
    float const numer = dot(normal, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;

        float3 const t = -trafo.rotation.r[0];

        float const u = dot(t, k / trafo.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 const b = -trafo.rotation.r[1];

        float const v = dot(b, k / trafo.scale_y());
        if (v > 1.f || v < -1.f) {
            return false;
        }

        isec.p     = p;
        isec.t     = t;
        isec.b     = b;
        isec.n     = normal;
        isec.geo_n = normal;
        isec.uv[0] = 0.5f * (u + 1.f);
        isec.uv[1] = 0.5f * (v + 1.f);

        isec.part = 0;

        ray.max_t() = hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect_p(Ray const& ray, Transformation const& trafo,
                            Node_stack& /*nodes*/) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const denom = -dot(n, ray.direction);
    float const numer = dot(n, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;

        float3 const t = -trafo.rotation.r[0];

        float const u = dot(t, k / trafo.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 const b = -trafo.rotation.r[1];

        float const v = dot(b, k / trafo.scale_y());

        return (v <= 1.f) & (v >= -1.f);
    }

    return false;
}

bool Rectangle::thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                                Filter filter, Worker& worker, float3& ta) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const denom = -dot(n, ray.direction);
    float const numer = dot(n, ray.origin) - d;
    float const hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;

        float3 const t = -trafo.rotation.r[0];

        float const u = dot(t, k / trafo.scale_x());
        if (u > 1.f || u < -1.f) {
            ta = float3(1.f);
            return true;
        }

        float3 const b = -trafo.rotation.r[1];

        float const v = dot(b, k / trafo.scale_y());
        if (v > 1.f || v < -1.f) {
            ta = float3(1.f);
            return true;
        }

        float2 const uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
        return worker.scene().prop_material(entity, 0)->visibility(ray.direction, n, uv, filter,
                                                                   worker, ta);
    }

    ta = float3(1.f);
    return true;
}

bool Rectangle::sample(uint32_t part, uint32_t /*variant*/, float3_p p, float3_p /*n*/,
                       Transformation const& trafo, float area, bool two_sided,
                       bool /*total_sphere*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                       Sample_to& sample) const {
    float2 const r2 = sampler.sample_2D(rng, sampler_d);

    return Rectangle::sample(part, p, r2, trafo, area, two_sided, sample);
}

bool Rectangle::sample(uint32_t part, uint32_t /*variant*/, Transformation const& trafo, float area,
                       bool two_sided, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                       float2 importance_uv, AABB const& bounds, Sample_from& sample) const {
    float2 const r0 = sampler.sample_2D(rng, sampler_d);

    return Rectangle::sample(part, r0, trafo, area, two_sided, importance_uv, bounds, sample);
}

float Rectangle::pdf(uint32_t /*variant*/, Ray const&                    ray, float3_p /*n*/,
                     Intersection const& /*isec*/, Transformation const& trafo, float area,
                     bool two_sided, bool /*total_sphere*/) const {
    float3 const n = trafo.rotation.r[2];

    float c = -dot(n, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t() * ray.max_t();
    return sl / (c * area);
}

float Rectangle::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                            Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Rectangle::sample(uint32_t /*part*/, float3_p p, float2 uv, Transformation const& trafo,
                       float area, bool two_sided, Sample_to& sample) const {
    float3 const ls(-2.f * uv + 1.f, 0.f);
    float3 const ws = trafo.object_to_world_point(ls);
    float3       wn = trafo.rotation.r[2];

    if (two_sided && dot(wn, ws - p) > 0.f) {
        wn *= -1.f;
    }

    float3 const axis = offset_ray(ws, wn) - p;
    float const  sl   = squared_length(axis);
    float const  t    = std::sqrt(sl);
    float3 const dir  = axis / t;
    float const  c    = -dot(wn, dir);

    if (c < Dot_min) {
        return false;
    }

    sample = Sample_to(dir, wn, float3(uv), sl / (c * area), t);

    return true;
}

bool Rectangle::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                              Transformation const& /*trafo*/, float /*volume*/,
                              Sample_to& /*sample*/) const {
    return false;
}

bool Rectangle::sample(uint32_t /*part*/, float2 uv, Transformation const& trafo, float area,
                       bool /*two_sided*/, float2 importance_uv, AABB const& /*bounds*/,
                       Sample_from& sample) const {
    float3 const ls(-2.f * uv + 1.f, 0.f);
    float3 const ws = trafo.object_to_world_point(ls);
    float3 const wn = trafo.rotation.r[2];

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, trafo.rotation);

    sample = Sample_from(offset_ray(ws, wn), wn, dir, uv, importance_uv, 1.f / (Pi * area));

    return true;
}

float Rectangle::pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& trafo,
                        float area, bool two_sided) const {
    return pdf(0, ray, float3(0.f), isec, trafo, area, two_sided, false);
}

float Rectangle::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Rectangle::area(uint32_t /*part*/, float3_p scale) const {
    return 4.f * scale[0] * scale[1];
}

float Rectangle::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Rectangle::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

float4 Rectangle::cone(uint32_t /*part*/) const {
    return float4(0.f, 0.f, 1.f, 1.f);
}

}  // namespace scene::shape
