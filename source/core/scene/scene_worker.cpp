#include "scene_worker.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "material/material_sample.hpp"
#include "material/sampler_cache.hpp"
#include "prop/interface_stack.inl"
#include "prop/prop.hpp"
#include "prop/prop_intersection.inl"
#include "scene.hpp"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"
#include "scene_constants.hpp"
#include "scene_ray.inl"
#include "shape/node_stack.inl"

namespace scene {

static material::Sampler_cache const Sampler_cache;

using Material_sample    = scene::material::Sample;
using Texture_sampler_2D = image::texture::Sampler_2D;
using Texture_sampler_3D = image::texture::Sampler_3D;

Worker::Worker() = default;

Worker::~Worker() = default;

void Worker::init(Scene const& scene, Camera const& camera) {
    scene_  = &scene;
    camera_ = &camera;
}

void Worker::init_rng(uint64_t sequence) {
    rng_.start(0, sequence);
}

bool Worker::resolve_mask(Ray& ray, Intersection& intersection, Filter filter) {
    float const start_min_t = ray.min_t();

    float opacity = intersection.opacity(ray.time, filter, *this);

    while (opacity < 1.f) {
        if (opacity > 0.f && opacity > rng_.random_float()) {
            ray.min_t() = start_min_t;
            return true;
        }

        // Slide along ray until opaque surface is found
        ray.min_t() = offset_f(ray.max_t());
        ray.max_t() = scene::Ray_max_t;
        if (!intersect(ray, intersection)) {
            ray.min_t() = start_min_t;
            return false;
        }

        opacity = intersection.opacity(ray.time, filter, *this);
    }

    ray.min_t() = start_min_t;
    return true;
}

Texture_sampler_2D const& Worker::sampler_2D(uint32_t key, Filter filter) const {
    return Sampler_cache.sampler_2D(key, filter);
}

Texture_sampler_3D const& Worker::sampler_3D(uint32_t key, Filter filter) const {
    return Sampler_cache.sampler_3D(key, filter);
}

void Worker::reset_interface_stack(Interface_stack const& stack) {
    interface_stack_ = stack;
}

float Worker::ior_outside(float3 const& wo, Intersection const& intersection) const {
    if (intersection.same_hemisphere(wo)) {
        return interface_stack_.top_ior(*this);
    }

    return interface_stack_.peek_ior(intersection, *this);
}

void Worker::interface_change(float3 const& dir, Intersection const& intersection) {
    if (bool const leave = intersection.same_hemisphere(dir); leave) {
        interface_stack_.remove(intersection);
    } else if (interface_stack_.straight(*this) | (intersection.material(*this)->ior() > 1.f)) {
        interface_stack_.push(intersection);
    }
}

material::IoR Worker::interface_change_ior(float3 const& dir, Intersection const& intersection) {
    float const inter_ior = intersection.material(*this)->ior();

    if (bool const leave = intersection.same_hemisphere(dir); leave) {
        auto const ior = material::IoR{interface_stack_.peek_ior(intersection, *this), inter_ior};

        interface_stack_.remove(intersection);

        return ior;
    }

    auto const ior = material::IoR{inter_ior, interface_stack_.top_ior(*this)};

    if (interface_stack_.straight(*this) | (inter_ior > 1.f)) {
        interface_stack_.push(intersection);
    }

    return ior;
}

Material_sample const& Worker::sample_material(Ray const& ray, float3 const& wo, float3 const& wo1,
                                               Intersection const& intersection, Filter filter,
                                               bool avoid_caustics, bool straight_border,
                                               Sampler& sampler) {
    auto material = intersection.material(*this);

    float3 const wi = ray.direction;

    if (((!intersection.subsurface) & straight_border & (material->ior() > 1.f)) &&
        intersection.same_hemisphere(wi)) {
        auto& sample = Worker::sample<material::null::Sample>();

        float3 const geo_n = intersection.geo.geo_n;
        float3 const n     = intersection.geo.n;

        float const vbh = material->border(wi, n);
        float const nsc = material::non_symmetry_compensation(wi, wo1, geo_n, n);

        sample.set_basis(geo_n, n, wo);
        sample.factor_ = nsc * vbh;

        return sample;
    }

    return intersection.sample(wo, ray, filter, avoid_caustics, sampler, *this);
}

}  // namespace scene
