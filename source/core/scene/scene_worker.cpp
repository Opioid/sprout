#include "scene_worker.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "material/material.inl"
#include "material/material_helper.hpp"
#include "material/material_sample.inl"
#include "material/null/null_sample.hpp"
#include "material/sampler_cache.hpp"
#include "prop/interface_stack.inl"
#include "prop/prop.hpp"
#include "prop/prop_intersection.inl"
#include "scene.hpp"
#include "scene_constants.hpp"
#include "scene_ray.inl"
#include "shape/node_stack.inl"
#include "shape/shape.hpp"

namespace scene {

static material::Sampler_cache const Sampler_cache;

using Transformation     = entity::Composed_transformation;
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

bool Worker::resolve_mask(Ray& ray, Intersection& isec, Filter filter) {
    float const start_min_t = ray.min_t();

    float opacity = isec.opacity(ray.time, filter, *this);

    while (opacity < 1.f) {
        if (opacity > 0.f && opacity > rng_.random_float()) {
            ray.min_t() = start_min_t;
            return true;
        }

        // Slide along ray until opaque surface is found
        ray.min_t() = offset_f(ray.max_t());
        ray.max_t() = scene::Ray_max_t;
        if (!intersect(ray, isec)) {
            ray.min_t() = start_min_t;
            return false;
        }

        opacity = isec.opacity(ray.time, filter, *this);
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

float Worker::ior_outside(float3_p wo, Intersection const& isec) const {
    if (isec.same_hemisphere(wo)) {
        return interface_stack_.top_ior(*this);
    }

    return interface_stack_.peek_ior(isec, *this);
}

void Worker::interface_change(float3_p dir, Intersection const& isec) {
    if (bool const leave = isec.same_hemisphere(dir); leave) {
        interface_stack_.remove(isec);
    } else if (interface_stack_.straight(*this) | (isec.material(*this)->ior() > 1.f)) {
        interface_stack_.push(isec);
    }
}

material::IoR Worker::interface_change_ior(float3_p dir, Intersection const& isec) {
    float const inter_ior = isec.material(*this)->ior();

    if (bool const leave = isec.same_hemisphere(dir); leave) {
        auto const ior = material::IoR{interface_stack_.peek_ior(isec, *this), inter_ior};

        interface_stack_.remove(isec);

        return ior;
    }

    auto const ior = material::IoR{inter_ior, interface_stack_.top_ior(*this)};

    if (interface_stack_.straight(*this) | (inter_ior > 1.f)) {
        interface_stack_.push(isec);
    }

    return ior;
}

Material_sample const& Worker::sample_material(Ray const& ray, float3_p wo, float3_p wo1,
                                               Intersection const& isec, Filter filter, float alpha,
                                               bool avoid_caustics, bool straight_border,
                                               Sampler& sampler) {
    auto material = isec.material(*this);

    float3 const wi = ray.direction;

    if (((!isec.subsurface) & straight_border & (material->ior() > 1.f)) &&
        isec.same_hemisphere(wi)) {
        auto& sample = Worker::sample<material::null::Sample>();

        float3 const geo_n = isec.geo.geo_n;
        float3 const n     = isec.geo.n;

        float const vbh = material->border(wi, n);
        float const nsc = material::non_symmetry_compensation(wi, wo1, geo_n, n);

        sample.set_common(geo_n, n, wo, alpha);
        sample.factor_ = nsc * vbh;

        return sample;
    }

    return isec.sample(wo, ray, filter, alpha, avoid_caustics, sampler, *this);
}

// https://blog.yiningkarlli.com/2018/10/bidirectional-mipmap.html
static float4 calculate_screenspace_differential(float3_p p, float3_p n, Ray_differential const& rd,
                                                 float3_p dpdu, float3_p dpdv) {
    // Compute offset-ray isec points with tangent plane
    float const d = dot(n, p);

    float const tx = -(dot(n, rd.x_origin) - d) / dot(n, rd.x_direction);
    float const ty = -(dot(n, rd.y_origin) - d) / dot(n, rd.y_direction);

    float3 const px = rd.x_origin + tx * rd.x_direction;
    float3 const py = rd.y_origin + ty * rd.y_direction;

    // Compute uv offsets at offset-ray isec points
    // Choose two dimensions to use for ray offset computations
    int2 dim;
    if (std::abs(n[0]) > std::abs(n[1]) && std::abs(n[0]) > std::abs(n[2])) {
        dim = int2(1, 2);
    } else if (std::abs(n[1]) > std::abs(n[2])) {
        dim = int2(0, 2);
    } else {
        dim = int2(0, 1);
    }

    // Initialize A, bx, and by matrices for offset computation
    float const a[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]}, {dpdu[dim[1]], dpdv[dim[1]]}};

    float2 const bx(px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]);
    float2 const by(py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]);

    float const det = a[0][0] * a[1][1] - a[0][1] * a[1][0];

    if (std::abs(det) < 1e-10f) {
        return float4(0.f);
    }

    float const dudx = (a[1][1] * bx[0] - a[0][1] * bx[1]) / det;
    float const dvdx = (a[0][0] * bx[1] - a[1][0] * bx[0]) / det;

    float const dudy = (a[1][1] * by[0] - a[0][1] * by[1]) / det;
    float const dvdy = (a[0][0] * by[1] - a[1][0] * by[0]) / det;

    return float4(dudx, dvdx, dudy, dvdy);
}

float4 Worker::screenspace_differential(Intersection const& isec, uint64_t time) const {
    Ray_differential const rd = camera_->calculate_ray_differential(isec.geo.p, time, *scene_);

    Transformation temp;
    auto const&    trafo = scene_->prop_transformation_at(isec.prop, time, temp);

    auto const ds = scene_->prop_shape(isec.prop)->differential_surface(isec.geo.primitive);

    float3 const dpdu_w = trafo.object_to_world_vector(ds.dpdu);
    float3 const dpdv_w = trafo.object_to_world_vector(ds.dpdv);

    return calculate_screenspace_differential(isec.geo.p, isec.geo.geo_n, rd, dpdu_w, dpdv_w);
}

float4 Worker::screenspace_differential(Renderstate const& rs, uint64_t time) const {
    Ray_differential const rd = camera_->calculate_ray_differential(rs.p, time, *scene_);

    Transformation temp;
    auto const&    trafo = scene_->prop_transformation_at(rs.prop, time, temp);

    auto const ds = scene_->prop_shape(rs.prop)->differential_surface(rs.primitive);

    float3 const dpdu_w = trafo.object_to_world_vector(ds.dpdu);
    float3 const dpdv_w = trafo.object_to_world_vector(ds.dpdv);

    return calculate_screenspace_differential(rs.p, rs.geo_n, rd, dpdu_w, dpdv_w);
}

}  // namespace scene
