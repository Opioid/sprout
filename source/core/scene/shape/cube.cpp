#include "cube.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/composed_transformation.inl"
#include "scene/material/material.hpp"
#include "scene/material/material.inl"
#include "scene/ray_offset.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Cube::Cube() : Shape(Properties(Property::Finite, Property::Analytical)) {}

AABB Cube::aabb() const {
    return AABB(float3(-1.f), float3(1.f));
}

bool Cube::intersect(Ray& ray, Transformation const& trafo, Worker& /*worker*/, Interpolation ipo,
                     Intersection& isec) const {
    float3 const local_origin = trafo.world_to_object_point(ray.origin);
    float3 const local_dir    = trafo.world_to_object_vector(ray.direction);

    math::ray const local_ray(local_origin, local_dir, ray.min_t(), ray.max_t());

    AABB const aabb(float3(-1.f), float3(1.f));

    float hit_t;
    if (!aabb.intersect_p(local_ray, hit_t)) {
        return false;
    }

    if (hit_t > ray.max_t()) {
        return false;
    }

    ray.max_t() = hit_t;

    isec.p = ray.point(hit_t);

    float3 const local_p  = local_ray.point(hit_t);
    float3 const distance = abs(1.f - abs(local_p));

    uint32_t const i = index_min_component(distance);

    float const s = copysign1(local_p[i]);

    float3 const n = s * trafo.rotation.r[i];

    isec.part = 0;

    isec.geo_n = n;
    isec.n     = n;

    if (Interpolation::Normal != ipo) {
        auto const tb = orthonormal_basis(n);

        isec.t = tb.a;
        isec.b = tb.b;
    }

    return true;
}

bool Cube::intersect_p(Ray const& ray, Transformation const& trafo, Worker& /*worker*/) const {
    float3 const local_origin = trafo.world_to_object_point(ray.origin);
    float3 const local_dir    = trafo.world_to_object_vector(ray.direction);

    math::ray const local_ray(local_origin, local_dir, ray.min_t(), ray.max_t());

    AABB const aabb(float3(-1.f), float3(1.f));

    float hit_t;
    if (!aabb.intersect_p(local_ray, hit_t)) {
        return false;
    }

    if (hit_t > ray.max_t()) {
        return false;
    }

    return true;
}

bool Cube::visibility(Ray const& /*ray*/, Transformation const& /*trafo*/, uint32_t /*entity*/,
                      Filter /*filter*/, Worker& /*worker*/, float3& vis) const {
    vis = float3(1.f);
    return true;
}

bool Cube::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p p, float3_p /*n*/,
                  Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                  Sample_to& sample) const {
    float3 const axis                = trafo.position - p;
    float const  axis_squared_length = squared_length(axis);
    float const  radius              = trafo.scale_x();
    float const  radius_square       = radius * radius;
    float const  sin_theta_max2      = radius_square / axis_squared_length;
    float        cos_theta_max       = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
    cos_theta_max                    = std::min(0.99999995f, cos_theta_max);

    float const  axis_length = std::sqrt(axis_squared_length);
    float3 const z           = axis / axis_length;

    auto const [x, y] = orthonormal_basis(z);

    float2 const r2  = sampler.sample_2D(rng, sampler_d);
    float3 const dir = math::sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

    float const d = axis_length - radius;  // this is not accurate

    sample = Sample_to(dir, z, float3(0.f), math::cone_pdf_uniform(cos_theta_max), offset_b(d));

    return true;
}

bool Cube::sample(uint32_t /*part*/, uint32_t /*variant*/, Transformation const& /*trafo*/,
                  float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/, RNG& /*rng*/,
                  uint32_t /*sampler_d*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const {
    return false;
}

bool Cube::sample_volume(uint32_t /*part*/, float3_p p, Transformation const& trafo, float volume,
                         Sampler& sampler, RNG& rng, uint32_t sampler_d, Sample_to& sample) const {
    float2 const r2 = sampler.sample_2D(rng, sampler_d);
    float const  r1 = sampler.sample_1D(rng, sampler_d);

    float3 const r3(r2, r1);
    float3 const xyz  = 2.f * (r3 - 0.5f);
    float3 const wp   = trafo.object_to_world_point(xyz);
    float3 const axis = wp - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    sample = Sample_to(axis / t, float3(0.f), r3, sl / volume, t);

    return true;
}

float Cube::pdf(uint32_t /*variant*/, Ray const& ray, float3_p /*n*/, Intersection const& /*isec*/,
                Transformation const& trafo, float /*area*/, bool /*two_sided*/,
                bool /*total_sphere*/) const {
    float3 const axis                = trafo.position - ray.origin;
    float const  axis_squared_length = squared_length(axis);
    float const  radius              = trafo.scale_x();
    float const  radius_square       = radius * radius;
    float const  sin_theta_max2      = radius_square / axis_squared_length;
    float        cos_theta_max       = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
    cos_theta_max                    = std::min(0.99999995f, cos_theta_max);

    return math::cone_pdf_uniform(cos_theta_max);
}

float Cube::pdf_volume(Ray const& ray, Intersection const& /*isec*/,
                       Transformation const& /*trafo*/, float volume) const {
    float const sl = ray.max_t() * ray.max_t();
    return sl / (volume);
}

bool Cube::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/, Transformation const& /*trafo*/,
                  float /*area*/, bool /*two_sided*/, Sample_to& /*sample*/) const {
    return false;
}

bool Cube::sample_volume(uint32_t /*part*/, float3_p p, float3_p uvw, Transformation const& trafo,
                         float volume, Sample_to& sample) const {
    float3 const xyz  = 2.f * (uvw - 0.5f);
    float3 const wp   = trafo.object_to_world_point(xyz);
    float3 const axis = wp - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    sample = Sample_to(axis / t, float3(0.f), uvw, sl / volume, t);

    return true;
}

bool Cube::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/, float /*area*/,
                  bool /*two_sided*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const {
    return false;
}

float Cube::pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& /*trafo*/,
                   float area, bool /*two_sided*/) const {
    //	float3 xyz = transform_vector_transposed(wn, trafo.rotation);
    //	uv[0] = -std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    //	uv[1] =  std::acos(xyz[1]) * Pi_inv;

    //	// sin_theta because of the uv weight
    //	float sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(isec.uv[1] * Pi);

    float const sl = ray.max_t() * ray.max_t();
    float const c  = -dot(isec.geo_n, ray.direction);
    return sl / (c * area * sin_theta);
}

float Cube::uv_weight(float2 uv) const {
    float const sin_theta = std::sin(uv[1] * Pi);

    if (0.f == sin_theta) {
        // this case never seemed to be an issue?!
        return 0.f;
    }

    return 1.f / sin_theta;
}

float Cube::area(uint32_t /*part*/, float3_p scale) const {
    float3 const d = 2.f * scale;
    return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

float Cube::volume(uint32_t /*part*/, float3_p scale) const {
    float3 const d = 2.f * scale;
    return d[0] * d[1] * d[2];
}

Shape::Differential_surface Cube::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
