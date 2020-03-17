#include "rectangle.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Rectangle::Rectangle() = default;

float3 Rectangle::object_to_texture_point(float3 const& p) const {
    return (p - float3(-1.f)) * (1.f / float3(2.f, 2.f, 0.2f));
}

float3 Rectangle::object_to_texture_vector(float3 const& v) const {
    return v * (1.f / float3(2.f, 2.f, 0.2f));
}

AABB Rectangle::transformed_aabb(float4x4 const& m) const {
    return AABB(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f)).transform(m);
}

bool Rectangle::intersect(Ray& ray, Transformation const&           transformation,
                          Node_stack& /*node_stack*/, Intersection& intersection) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale_y());
        if (v > 1.f || v < -1.f) {
            return false;
        }

        intersection.p     = p;
        intersection.t     = t;
        intersection.b     = b;
        intersection.n     = normal;
        intersection.geo_n = normal;
        intersection.uv[0] = 0.5f * (u + 1.f);
        intersection.uv[1] = 0.5f * (v + 1.f);

        intersection.part = 0;

        ray.max_t() = hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect_nsf(Ray& ray, Transformation const&           transformation,
                              Node_stack& /*node_stack*/, Intersection& intersection) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale_y());
        if (v > 1.f || v < -1.f) {
            return false;
        }

        intersection.p     = p;
        intersection.geo_n = normal;
        intersection.uv[0] = 0.5f * (u + 1.f);
        intersection.uv[1] = 0.5f * (v + 1.f);

        intersection.part = 0;

        ray.max_t() = hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect(Ray& ray, Transformation const&      transformation,
                          Node_stack& /*node_stack*/, Normals& normals) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale_y());
        if (v > 1.f || v < -1.f) {
            return false;
        }

        ray.max_t() = hit_t;

        normals.geo_n = normal;
        normals.n     = normal;

        return true;
    }

    return false;
}

bool Rectangle::intersect_p(Ray const& ray, Transformation const& transformation,
                            Node_stack& /*node_stack*/) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 const b = -transformation.rotation.r[1];

        float const v = dot(b, k / transformation.scale_y());

        return (v <= 1.f) & (v >= -1.f);
    }

    return false;
}

float Rectangle::visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                            Filter filter, Worker& worker) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            return 1.f;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale_y());
        if (v > 1.f || v < -1.f) {
            return 1.f;
        }

        float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
        return 1.f - worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
    }

    return 1.f;
}

bool Rectangle::thin_absorption(Ray const& ray, Transformation const& transformation,
                                uint32_t entity, Filter filter, Worker& worker, float3& ta) const {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale_x());
        if (u > 1.f || u < -1.f) {
            ta = float3(1.f);
            return true;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale_y());
        if (v > 1.f || v < -1.f) {
            ta = float3(1.f);
            return true;
        }

        float2 const uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
        ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, normal, uv,
                                                                      ray.time, filter, worker);
        return true;
    }

    ta = float3(1.f);
    return true;
}

bool Rectangle::sample(uint32_t part, float3 const& p, Transformation const& transformation,
                       float area, bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                       Sample_to& sample) const {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);

    return Rectangle::sample(part, p, r2, transformation, area, two_sided, sample);
}

bool Rectangle::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                       bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                       float2 importance_uv, AABB const& /*bounds*/, Sample_from& sample) const {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);

    float3 const ls(-2.f * r0 + 1.f, 0.f);
    float3 const ws = transformation.object_to_world_point(ls);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, transformation.rotation);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / (Pi * area);

    return true;
}

float Rectangle::pdf(Ray const&            ray, Intersection const& /*intersection*/,
                     Transformation const& transformation, float area, bool two_sided,
                     bool /*total_sphere*/) const {
    float3 const& normal = transformation.rotation.r[2];

    float c = -dot(normal, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t() * ray.max_t();
    return sl / (c * area);
}

float Rectangle::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                            Transformation const& /*transformation*/, float /*volume*/) const {
    return 0.f;
}

bool Rectangle::sample(uint32_t /*part*/, float3 const& p, float2 uv,
                       Transformation const& transformation, float area, bool two_sided,
                       Sample_to& sample) const {
    float3 const ls(-2.f * uv + 1.f, 0.f);
    float3 const ws = transformation.object_to_world_point(ls);

    float3 const axis = ws - p;
    float const  sl   = squared_length(axis);
    float const  t    = std::sqrt(sl);

    float3 const dir = axis / t;

    float3 const wn = transformation.rotation.r[2];

    float c = -dot(wn, dir);

    if (two_sided) {
        c = std::abs(c);
    }

    if (c <= Dot_min) {
        return false;
    }

    sample = Sample_to(dir, float3(uv), sl / (c * area), offset_b(t));

    return true;
}

bool Rectangle::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                       Transformation const& /*transformation*/, float /*volume*/,
                       Sample_to& /*sample*/) const {
    return false;
}

bool Rectangle::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                       float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                       AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Rectangle::pdf_uv(Ray const& ray, Intersection const& intersection,
                        Transformation const& transformation, float area, bool two_sided) const {
    return pdf(ray, intersection, transformation, area, two_sided, false);
}

float Rectangle::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Rectangle::area(uint32_t /*part*/, float3 const& scale) const {
    return 4.f * scale[0] * scale[1];
}

float Rectangle::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

size_t Rectangle::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::shape
