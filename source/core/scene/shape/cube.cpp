#include "cube.hpp"
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

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Cube::Cube() = default;

float3 Cube::object_to_texture_point(float3 const& p) const {
    return (p - float3(-1.f)) * (1.f / float3(2.f));
}

float3 Cube::object_to_texture_vector(float3 const& v) const {
    return v * (1.f / float3(2.f));
}

AABB Cube::transformed_aabb(float4x4 const& m) const {
    return AABB(float3(-1.f), float3(1.f)).transform(m);
}

bool Cube::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     Intersection& intersection) const {
    bool const hit = intersect_nsf(ray, transformation, node_stack, intersection);

    intersection.n = intersection.geo_n;

    auto const tb = orthonormal_basis(intersection.geo_n);

    intersection.t = tb.a;
    intersection.b = tb.b;

    return hit;
}

bool Cube::intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                         Intersection& intersection) const {
    float3 const local_origin = transformation.world_to_object_point(ray.origin);
    float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

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

    intersection.p = ray.point(hit_t);

    float3 const local_p = local_ray.point(hit_t);

    float3 const distance = abs(1.f - abs(local_p));

    uint32_t const i = index_min_component(distance);

    float const s = copysign1(local_p[i]);

    intersection.geo_n = s * transformation.rotation.r[i];

    intersection.part = 0;

    return true;
}

bool Cube::intersect(Ray& ray, Transformation const& transformation, Node_stack& /*node_stack*/,
                     Normals& normals) const {
    float3 const local_origin = transformation.world_to_object_point(ray.origin);
    float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

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

    float3 const local_p = local_ray.point(hit_t);

    float3 const distance = abs(1.f - abs(local_p));

    uint32_t const i = index_min_component(distance);

    float const s = copysign1(local_p[i]);

    float3 const wn = s * transformation.rotation.r[i];

    normals.geo_n = wn;
    normals.n     = wn;

    return true;
}

bool Cube::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& /*node_stack*/) const {
    float3 const local_origin = transformation.world_to_object_point(ray.origin);
    float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

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

float Cube::visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                       Filter filter, Worker& worker) const {
    float3 v      = transformation.position - ray.origin;
    float  b      = dot(v, ray.direction);
    float  radius = transformation.scale_x();
    float  det    = (b * b) - dot(v, v) + (radius * radius);

    if (det > 0.f) {
        float dist = std::sqrt(det);
        float t0   = b - dist;

        if (t0 > ray.min_t() && t0 < ray.max_t()) {
            float3 n = normalize(ray.point(t0) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return 1.f -
                   worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
        }

        float t1 = b + dist;

        if (t1 > ray.min_t() && t1 < ray.max_t()) {
            float3 n = normalize(ray.point(t1) - transformation.position);

            float3 xyz = transform_vector_transposed(transformation.rotation, n);
            xyz        = normalize(xyz);

            float2 uv = float2(-std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f,
                               std::acos(xyz[1]) * Pi_inv);

            return 1.f -
                   worker.scene().prop_material(entity, 0)->opacity(uv, ray.time, filter, worker);
        }
    }

    return 1.f;
}

bool Cube::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                           uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/,
                           float3& ta) const {
    ta = float3(1.f);
    return true;
}

bool Cube::sample(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                  float /*area*/, bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                  Sample_to& sample) const {
    float3 const axis                = transformation.position - p;
    float const  axis_squared_length = squared_length(axis);
    float const  radius              = transformation.scale_x();
    float const  radius_square       = radius * radius;
    float const  sin_theta_max2      = radius_square / axis_squared_length;
    float        cos_theta_max       = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
    cos_theta_max                    = std::min(0.99999995f, cos_theta_max);

    float const  axis_length = std::sqrt(axis_squared_length);
    float3 const z           = axis / axis_length;

    auto const [x, y] = orthonormal_basis(z);

    float2 const r2  = sampler.generate_sample_2D(sampler_dimension);
    float3 const dir = math::sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

    float const d = axis_length - radius;  // this is not accurate

    sample = Sample_to(dir, float3(0.f), math::cone_pdf_uniform(cos_theta_max), offset_b(d));

    return true;
}

bool Cube::sample(uint32_t /*part*/, Transformation const& /*transformation*/, float /*area*/,
                  bool /*two_sided*/, Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                  float2 /*importance_uv*/, AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

bool Cube::sample_volume(uint32_t /*part*/, float3 const& p, Transformation const& transformation,
                         float volume, Sampler& sampler, uint32_t sampler_dimension,
                         Sample_to& sample) const {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float const  r1 = sampler.generate_sample_1D(sampler_dimension);

    float3 const r3(r2, r1);
    float3 const xyz  = 2.f * (r3 - 0.5f);
    float3 const wp   = transformation.object_to_world_point(xyz);
    float3 const axis = wp - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    sample = Sample_to(axis / t, r3, sl / volume, t);

    return true;
}

float Cube::pdf(Ray const&            ray, Intersection const& /*intersection*/,
                Transformation const& transformation, float /*area*/, bool /*two_sided*/,
                bool /*total_sphere*/) const {
    float3 const axis                = transformation.position - ray.origin;
    float const  axis_squared_length = squared_length(axis);
    float const  radius              = transformation.scale_x();
    float const  radius_square       = radius * radius;
    float const  sin_theta_max2      = radius_square / axis_squared_length;
    float        cos_theta_max       = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
    cos_theta_max                    = std::min(0.99999995f, cos_theta_max);

    return math::cone_pdf_uniform(cos_theta_max);
}

float Cube::pdf_volume(Ray const& ray, Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float volume) const {
    float const sl = ray.max_t() * ray.max_t();
    return sl / (volume);
}

bool Cube::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Cube::sample(uint32_t /*part*/, float3 const& p, float3 const& uvw,
                  Transformation const& transformation, float volume, Sample_to& sample) const {
    float3 const xyz  = 2.f * (uvw - 0.5f);
    float3 const wp   = transformation.object_to_world_point(xyz);
    float3 const axis = wp - p;

    float const sl = squared_length(axis);
    float const t  = std::sqrt(sl);

    sample = Sample_to(axis / t, uvw, sl / volume, t);

    return true;
}

bool Cube::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Cube::pdf_uv(Ray const& ray, Intersection const&             intersection,
                   Transformation const& /*transformation*/, float area, bool /*two_sided*/) const {
    //	float3 xyz = transform_vector_transposed(wn, transformation.rotation);
    //	uv[0] = -std::atan2(xyz[0], xyz[2]) * (Pi_inv * 0.5f) + 0.5f;
    //	uv[1] =  std::acos(xyz[1]) * Pi_inv;

    //	// sin_theta because of the uv weight
    //	float sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

    float const sin_theta = std::sin(intersection.uv[1] * Pi);

    float const sl = ray.max_t() * ray.max_t();
    float const c  = -dot(intersection.geo_n, ray.direction);
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

float Cube::area(uint32_t /*part*/, float3 const& scale) const {
    float3 const d = 2.f * scale;
    return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

float Cube::volume(uint32_t /*part*/, float3 const& scale) const {
    float3 const d = 2.f * scale;
    return d[0] * d[1] * d[2];
}

size_t Cube::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::shape
