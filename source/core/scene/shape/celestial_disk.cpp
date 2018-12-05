#include "celestial_disk.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"

namespace scene::shape {

Celestial_disk::Celestial_disk() noexcept {
    aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Celestial_disk::intersect(Ray& ray, Transformation const&           transformation,
                               Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t = hit_t;

        intersection.p     = ray.point(hit_t);
        intersection.geo_n = n;
        intersection.t     = transformation.rotation.r[0];
        intersection.b     = transformation.rotation.r[1];
        intersection.n     = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        intersection.uv[0] = (dot(intersection.t, sk) + 1.f) * 0.5f;
        intersection.uv[1] = (dot(intersection.b, sk) + 1.f) * 0.5f;

        intersection.epsilon = 5e-4f;
        intersection.part    = 0;

        return true;
    }

    return false;
}

bool Celestial_disk::intersect_fast(Ray& ray, Transformation const&           transformation,
                                    Node_stack& /*node_stack*/, Intersection& intersection) const
    noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        float constexpr hit_t = Almost_ray_max_t;

        ray.max_t = hit_t;

        intersection.p     = ray.point(hit_t);
        intersection.geo_n = n;

        float3 const k  = ray.direction - n;
        float3 const sk = k / radius;

        intersection.uv[0] = (dot(intersection.t, sk) + 1.f) * 0.5f;
        intersection.uv[1] = (dot(intersection.b, sk) + 1.f) * 0.5f;

        intersection.epsilon = 5e-4f;
        intersection.part    = 0;

        return true;
    }

    return false;
}

bool Celestial_disk::intersect(Ray& ray, Transformation const&    transformation,
                               Node_stack& /*node_stack*/, float& epsilon) const noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        ray.max_t = Almost_ray_max_t;
        epsilon   = 5e-4f;
        return true;
    }

    return false;
}

bool Celestial_disk::intersect_p(Ray const& ray, Transformation const& transformation,
                                 Node_stack& /*node_stack*/) const noexcept {
    float3 const n = transformation.rotation.r[2];

    float const b = dot(n, ray.direction);

    if (b > 0.f || ray.max_t < Ray_max_t) {
        return false;
    }

    float const radius = transformation.scale[0];
    float const det    = (b * b) - dot(n, n) + (radius * radius);

    if (det > 0.f) {
        return true;
    }

    return false;
}

float Celestial_disk::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
                              Materials const& /*materials*/, Filter /*filter*/,
                              Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return 0.f;
}

float3 Celestial_disk::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                                       Materials const& /*materials*/, Filter /*filter*/,
                                       Worker const& /*worker*/) const noexcept {
    // Implementation for this is not really needed, so just skip it
    return float3(0.f);
}

bool Celestial_disk::sample(uint32_t /*part*/, float3 const& /*p*/,
                            Transformation const& transformation, float area, bool /*two_sided*/,
                            Sampler& sampler, uint32_t             sampler_dimension,
                            Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = math::sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = transformation.scale[0];

    float3 const ws = radius * transform_vector(transformation.rotation, ls);

    sample.wi = normalize(ws - transformation.rotation.r[2]);

    sample.pdf     = 1.f / area;
    sample.t       = Almost_ray_max_t;
    sample.epsilon = 5e-4f;

    return true;
}

bool Celestial_disk::sample(uint32_t /*part*/, Transformation const& transformation, float area,
                            bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                            AABB const&  bounds, Node_stack& /*node_stack*/,
                            Sample_from& sample) const noexcept {
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    float2 const xy = math::sample_disk_concentric(r2);

    float3 const ls = float3(xy, 0.f);

    float const radius = transformation.scale[0];

    float3 const ws = radius * transform_vector(transformation.rotation, ls);

    float3 const dir = -normalize(ws - transformation.rotation.r[2]);

    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);

    float const bounds_radius = length(bounds.halfsize());

    float3 const receciver_disk = math::sample_oriented_disk_concentric(
        r0, transformation.rotation.r[0], transformation.rotation.r[1]);

    float3 const p = bounds.position() + bounds_radius * (receciver_disk - dir);

    sample.dir     = dir;
    sample.p       = p;
    sample.pdf     = 1.f / ((1.f * Pi) * (area * bounds_radius * bounds_radius));
    sample.epsilon = 5e-4f;

    return true;
}

float Celestial_disk::pdf(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
                          Transformation const& /*transformation*/, float area, bool /*two_sided*/,
                          bool /*total_sphere*/) const noexcept {
    return 1.f / area;
}

bool Celestial_disk::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Celestial_disk::sample(uint32_t /*part*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                            Sample_from& /*sample*/) const noexcept {
    return false;
}

float Celestial_disk::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                             Transformation const& /*transformation*/, float area,
                             bool /*two_sided*/) const noexcept {
    return 1.f / area;
}

float Celestial_disk::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Celestial_disk::area(uint32_t /*part*/, float3 const& scale) const noexcept {
    float const radius = scale[0];
    return Pi * (radius * radius);
}

float Celestial_disk::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

bool Celestial_disk::is_finite() const noexcept {
    return false;
}

size_t Celestial_disk::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
