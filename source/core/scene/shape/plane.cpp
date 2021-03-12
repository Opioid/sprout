#include "plane.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Plane::Plane() : Shape(Properties(Property::Analytical)) {}

float3 Plane::object_to_texture_point(float3_p p) const {
    return p;
}

float3 Plane::object_to_texture_vector(float3_p v) const {
    return v;
}

AABB Plane::transformed_aabb(float4x4 const& /*m*/) const {
    return Empty_AABB;
}

bool Plane::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                      Intersection& isec) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;
        float3 const t = -trafo.rotation.r[0];
        float3 const b = -trafo.rotation.r[1];

        isec.p     = p;
        isec.t     = t;
        isec.b     = b;
        isec.n     = n;
        isec.geo_n = n;
        isec.uv[0] = dot(t, k);
        isec.uv[1] = dot(b, k);

        isec.part = 0;

        ray.max_t() = hit_t;
        return true;
    }

    return false;
}

bool Plane::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                          Intersection& isec) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;
        float3 const t = -trafo.rotation.r[0];
        float3 const b = -trafo.rotation.r[1];

        isec.p     = p;
        isec.geo_n = n;
        isec.uv[0] = dot(t, k);
        isec.uv[1] = dot(b, k);

        isec.part = 0;

        ray.max_t() = hit_t;
        return true;
    }

    return false;
}

bool Plane::intersect(Ray& ray, Transformation const& trafo, Node_stack& /*nodes*/,
                      Normals& normals) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        ray.max_t() = hit_t;

        normals.geo_n = n;
        normals.n     = n;

        return true;
    }

    return false;
}

bool Plane::intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& /*nodes*/) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    return (hit_t > ray.min_t()) & (hit_t < ray.max_t());
}

float Plane::visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                        Worker& worker) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;
        float2 const uv(dot(trafo.rotation.r[0], k), dot(trafo.rotation.r[1], k));

        return 1.f - worker.scene().prop_material(entity, 0)->opacity(uv, filter, worker);
    }

    return 1.f;
}

bool Plane::thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                            Filter filter, Worker& worker, float3& ta) const {
    float3 const n = trafo.rotation.r[2];

    float const d     = dot(n, trafo.position);
    float const hit_t = -(dot(n, ray.origin) - d) / dot(n, ray.direction);

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        float3 const p = ray.point(hit_t);
        float3 const k = p - trafo.position;
        float2 const uv(dot(trafo.rotation.r[0], k), dot(trafo.rotation.r[1], k));

        ta = worker.scene().prop_material(entity, 0)->thin_absorption(ray.direction, n, uv, filter,
                                                                      worker);
        return true;
    }

    ta = float3(1.f);
    return true;
}

bool Plane::sample(uint32_t /*part*/, float3_p /*p*/, float3_p /*n*/,
                   Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                   bool /*total_sphere*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                   uint32_t /*sampler_d*/, Sample_to& /*sample*/) const {
    return false;
}

bool Plane::sample(uint32_t /*part*/, Transformation const& /*trafo*/, float /*area*/,
                   bool /*two_sided*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                   uint32_t /*sampler_d*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                   Sample_from& /*sample*/) const {
    return false;
}

float Plane::pdf(Ray const& /*ray*/, float3_p /*n*/, Intersection const& /*isec*/,
                 Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                 bool /*total_sphere*/) const {
    return 0.f;
}

float Plane::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                        Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Plane::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/,
                   Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                   Sample_to& /*sample*/) const {
    return false;
}

bool Plane::sample(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                   Transformation const& /*trafo*/, float /*volume*/, Sample_to& /*sample*/) const {
    return false;
}

bool Plane::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/,
                   float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                   AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Plane::pdf_uv(Ray const& /*ray*/, Intersection const& /*isec*/,
                    Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/) const {
    return 0.f;
}

float Plane::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Plane::area(uint32_t /*part*/, float3_p /*scale*/) const {
    return 1.f;
}

float Plane::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Plane::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
