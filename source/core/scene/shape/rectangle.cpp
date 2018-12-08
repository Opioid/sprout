#include "rectangle.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Rectangle::Rectangle() noexcept {
    aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Rectangle::intersect(Ray& ray, Transformation const&           transformation,
                          Node_stack& /*node_stack*/, Intersection& intersection) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return false;
        }

        intersection.epsilon = 5e-4f * hit_t;

        intersection.p     = p;
        intersection.t     = t;
        intersection.b     = b;
        intersection.n     = normal;
        intersection.geo_n = normal;
        intersection.uv[0] = 0.5f * (u + 1.f);
        intersection.uv[1] = 0.5f * (v + 1.f);

        intersection.part = 0;

        ray.max_t = hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect_fast(Ray& ray, Transformation const&           transformation,
                               Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = dot(normal, transformation.position);
    float         denom  = -dot(normal, ray.direction);
    float         numer  = dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return false;
        }

        intersection.epsilon = 5e-4f * hit_t;

        intersection.p     = p;
        intersection.geo_n = normal;
        intersection.uv[0] = 0.5f * (u + 1.f);
        intersection.uv[1] = 0.5f * (v + 1.f);

        intersection.part = 0;

        ray.max_t = hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect(Ray& ray, Transformation const&    transformation,
                          Node_stack& /*node_stack*/, float& epsilon) const noexcept {
    float3 const& normal = transformation.rotation.r[2];
    float         d      = dot(normal, transformation.position);
    float         denom  = -dot(normal, ray.direction);
    float         numer  = dot(normal, ray.origin) - d;
    float         hit_t  = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return false;
        }

        ray.max_t = hit_t;
        epsilon   = 5e-4f * hit_t;
        return true;
    }

    return false;
}

bool Rectangle::intersect_p(Ray const& ray, Transformation const& transformation,
                            Node_stack& /*node_stack*/) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return false;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return false;
        }

        return true;
    }

    return false;
}

float Rectangle::opacity(Ray const& ray, Transformation const& transformation,
                         Materials const& materials, Filter filter, Worker const& worker) const
    noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return 0.f;
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return 0.f;
        }

        float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
        return materials[0]->opacity(uv, ray.time, filter, worker);
    }

    return 0.f;
}

float3 Rectangle::thin_absorption(Ray const& ray, Transformation const& transformation,
                                  Materials const& materials, Filter filter,
                                  Worker const& worker) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float d     = dot(normal, transformation.position);
    float denom = -dot(normal, ray.direction);
    float numer = dot(normal, ray.origin) - d;
    float hit_t = numer / denom;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        float3 p = ray.point(hit_t);
        float3 k = p - transformation.position;

        float3 t = -transformation.rotation.r[0];

        float u = dot(t, k / transformation.scale[0]);
        if (u > 1.f || u < -1.f) {
            return float3(0.f);
        }

        float3 b = -transformation.rotation.r[1];

        float v = dot(b, k / transformation.scale[1]);
        if (v > 1.f || v < -1.f) {
            return float3(0.f);
        }

        float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
        return materials[0]->thin_absorption(ray.direction, normal, uv, ray.time, filter, worker);
    }

    return float3(0.f);
}

bool Rectangle::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                       float area, bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                       Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = 2.f * r2 - float2(1.f);

    float3 const scale(transformation.scale.xy(), 1.f);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transform_vector(transformation.rotation, scale * ls);

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

    sample.wi      = wi;
    sample.pdf     = sl / (c * area);
    sample.t       = t;
    sample.epsilon = 5e-4f * t;

    return true;
}

bool Rectangle::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                       bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                       AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                       Sample_from& sample) const noexcept {
    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = 2.f * r0 - float2(1.f);

    float3 const scale(transformation.scale.xy(), 1.f);

    float3 const ls = float3(xy, 0.f);
    float3 const ws = transformation.position +
                      transform_vector(transformation.rotation, scale * ls);

    float2 const r1  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_hemisphere_cosine(r1, transformation.rotation);

    sample.p       = ws;
    sample.dir     = dir;
    sample.pdf     = 1.f / (Pi * area);
    sample.epsilon = 5e-4f;

    return true;
}

float Rectangle::pdf(Ray const&            ray, const shape::Intersection& /*intersection*/,
                     Transformation const& transformation, float area, bool two_sided,
                     bool /*total_sphere*/) const noexcept {
    float3 const& normal = transformation.rotation.r[2];

    float c = -dot(normal, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t * ray.max_t;
    return sl / (c * area);
}

float Rectangle::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                            Transformation const& /*transformation*/, float /*volume*/) const
    noexcept {
    return 0.f;
}

bool Rectangle::sample(uint32_t /*part*/, float3 const& p, float2 uv,
                       Transformation const& transformation, float area, bool two_sided,
                       Sample_to& sample) const noexcept {
    float3 ls(-2.f * uv[0] + 1.f, -2.f * uv[1] + 1.f, 0.f);
    float3 ws = transform_point(transformation.object_to_world, ls);

    float3 axis = ws - p;
    float  sl   = squared_length(axis);
    float  d    = std::sqrt(sl);

    float3 dir = axis / d;

    float3 wn = transformation.rotation.r[2];

    float c = -dot(wn, dir);

    if (two_sided) {
        c = std::abs(c);
    }

    if (c <= 0.f) {
        return false;
    }

    sample.wi  = dir;
    sample.uvw = float3(uv);
    sample.t   = d;
    // sin_theta because of the uv weight
    sample.pdf = sl / (c * area /** sin_theta*/);

    return true;
}

bool Rectangle::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                       Transformation const& /*transformation*/, float /*volume*/,
                       Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Rectangle::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                       float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                       uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                       Sample_from& /*sample*/) const noexcept {
    return false;
}

float Rectangle::pdf_uv(Ray const& ray, Intersection const& intersection,
                        Transformation const& transformation, float area, bool two_sided) const
    noexcept {
    return pdf(ray, intersection, transformation, area, two_sided, false);
}

float Rectangle::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Rectangle::area(uint32_t /*part*/, float3 const& scale) const noexcept {
    return 4.f * scale[0] * scale[1];
}

float Rectangle::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

size_t Rectangle::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
