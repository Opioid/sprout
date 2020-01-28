#include "triangle_mesh.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "triangle_intersection.hpp"

#ifdef SU_DEBUG
#include "scene/shape/shape_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::shape::triangle {

Mesh::Mesh() noexcept : distributions_(nullptr), part_materials_(nullptr) {}

Mesh::~Mesh() noexcept {
    memory::free_aligned(part_materials_);
    memory::destroy_aligned(distributions_, tree_.num_parts());
}

Tree& Mesh::tree() noexcept {
    return tree_;
}

void Mesh::allocate_parts(uint32_t num_parts) noexcept {
    tree_.allocate_parts(num_parts);

    distributions_ = memory::construct_array_aligned<Distribution>(num_parts);

    part_materials_ = memory::allocate_aligned<uint32_t>(num_parts);
}

void Mesh::set_material_for_part(uint32_t part, uint32_t material) noexcept {
    part_materials_[part] = material;
}

float3 Mesh::object_to_texture_point(float3 const& p) const noexcept {
    return (p - tree_.aabb().bounds[0]) / tree_.aabb().extent();
}

float3 Mesh::object_to_texture_vector(float3 const& v) const noexcept {
    return v / tree_.aabb().extent();
}

AABB Mesh::transformed_aabb(float4x4 const& m) const noexcept {
    return tree_.aabb().transform(m);
}

uint32_t Mesh::num_parts() const noexcept {
    return tree_.num_parts();
}

uint32_t Mesh::num_materials() const noexcept {
    uint32_t id = 0;

    for (uint32_t i = 0, len = num_parts(); i < len; ++i) {
        id = std::max(id, part_materials_[i]);
    }

    return id + 1;
}

uint32_t Mesh::part_id_to_material_id(uint32_t part) const noexcept {
    return part_materials_[part];
}

bool Mesh::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     shape::Intersection& intersection) const noexcept {
    /*
    math::ray tray(
        transform_point(transformation.world_to_object, ray.origin),
        transform_vector(transformation.world_to_object, ray.direction),
        ray.min_t,
        ray.max_t);

    Intersectioni pi;
    if (tree_.intersect(tray, node_stack, pi)) {
        ray.max_t        = tray.max_t;

        float epsilon = 3e-3f * tray.max_t;

        float3 p = tree_.interpolate_p(pi.uv, pi.index);

        float3 p_w = transform_point(transformation.object_to_world, p);

        float3 n;
        float3 t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);

        float3 geo_n = tree_.triangle_normal(pi.index);

        float bitangent_sign = tree_.triangle_bitangent_sign(pi.index);

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        float3 geo_n_w = transform_vector(transformation.rotation, geo_n);
        float3 n_w     = transform_vector(transformation.rotation, n);
        float3 t_w     = transform_vector(transformation.rotation, t);
        float3 b_w     = bitangent_sign * cross(n_w, t_w);

        intersection.p = p_w;
        intersection.t = t_w;
        intersection.b = b_w;
        intersection.n = n_w;
        intersection.geo_n = geo_n_w;
        intersection.uv      = uv;
        intersection.epsilon = epsilon;
        intersection.part    = material_index;

        return true;
    }

    return false;
    */

    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f ray_origin(ray.origin);
    ray_origin = transform_point(world_to_object, ray_origin);

    Simd3f ray_direction(ray.direction);
    ray_direction = transform_vector(world_to_object, ray_direction);

    Simd3f ray_inv_direction = reciprocal(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    scalar ray_min_t(ray.min_t);
    scalar ray_max_t(ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        ray.max_t = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);
        Simd3f t_w     = transform_vector(rotation, t);
        Simd3f b_w     = bitangent_sign * cross(n_w, t_w);

        intersection.p     = float3(p_w);
        intersection.t     = float3(t_w);
        intersection.b     = float3(b_w);
        intersection.n     = float3(n_w);
        intersection.geo_n = float3(geo_n_w);

        intersection.uv   = uv;
        intersection.part = material_index;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Mesh::intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                         shape::Intersection& intersection) const noexcept {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f ray_origin(ray.origin);
    ray_origin = transform_point(world_to_object, ray_origin);

    Simd3f ray_direction(ray.direction);
    ray_direction = transform_vector(world_to_object, ray_direction);

    Simd3f ray_inv_direction = reciprocal(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    scalar ray_min_t(ray.min_t);
    scalar ray_max_t(ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        ray.max_t = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);

        intersection.p     = float3(p_w);
        intersection.geo_n = float3(geo_n_w);
        intersection.uv    = uv;
        intersection.part  = material_index;

        return true;
    }

    return false;
}

bool Mesh::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     Normals& normals) const noexcept {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f ray_origin(ray.origin);
    ray_origin = transform_point(world_to_object, ray_origin);

    Simd3f ray_direction(ray.direction);
    ray_direction = transform_vector(world_to_object, ray_direction);

    Simd3f const ray_inv_direction = reciprocal(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    scalar const ray_min_t(ray.min_t);
    scalar       ray_max_t(ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        ray.max_t = ray_max_t.x();

        Simd3f n = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);

        normals.geo_n = float3(geo_n_w);
        normals.n     = float3(n_w);

        return true;
    }

    return false;
}

bool Mesh::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& node_stack) const noexcept {
    //	ray tray;
    //	tray.origin = transform_point(ray.origin, transformation.world_to_object);
    //	tray.set_direction(transform_vector(ray.direction, transformation.world_to_object));
    //	tray.min_t = ray.min_t;
    //	tray.max_t = ray.max_t;

    //	return tree_.intersect_p(tray, node_stack);

    Simd4x4f world_to_object(transformation.world_to_object);

    Simd3f ray_origin(ray.origin);
    ray_origin = transform_point(world_to_object, ray_origin);

    Simd3f ray_direction(ray.direction);
    ray_direction = transform_vector(world_to_object, ray_direction);

    Simd3f ray_inv_direction = reciprocal(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    scalar ray_min_t(ray.min_t);
    scalar ray_max_t(ray.max_t);

    return tree_.intersect_p(ray_origin, ray_direction, ray_inv_direction, ray_min_t, ray_max_t,
                             ray_signs, node_stack);
}

float Mesh::opacity(Ray const& ray, Transformation const& transformation, uint32_t entity,
                    Filter filter, Worker const& worker) const noexcept {
    math::ray tray;
    tray.origin = transformation.world_to_object_point(ray.origin);
    tray.set_direction(transformation.world_to_object_vector(ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.opacity(tray, ray.time, entity, filter, worker);
}

bool Mesh::thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                           Filter filter, Worker const& worker, float3& ta) const noexcept {
    math::ray tray;
    tray.origin = transformation.world_to_object_point(ray.origin);
    tray.set_direction(transformation.world_to_object_vector(ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.absorption(tray, ray.time, entity, filter, worker, ta);
}

bool Mesh::sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                  bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                  Node_stack& /*node_stack*/, Sample_to& sample) const noexcept {
    float const  r  = sampler.generate_sample_1D(sampler_dimension);
    float2 const r2 = sampler.generate_sample_2D(sampler_dimension);
    auto const   s  = distributions_[part].sample(r);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r2, sv, tc);
    float3 const v = transformation.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(transformation.rotation, sn);

    float3 const axis = v - p;
    float const  sl   = squared_length(axis);
    float const  d    = std::sqrt(sl);
    float3 const dir  = axis / d;

    float c = -dot(wn, dir);

    if (two_sided) {
        c = std::abs(c);
    }

    if (c <= 0.00001f) {
        return false;
    }

    sample.wi  = dir;
    sample.uvw = float3(tc);
    sample.pdf = sl / (c * area);
    sample.t   = offset_b(d);

    return true;
}

bool Mesh::sample(uint32_t part, Transformation const& transformation, float area,
                  bool /*two_sided*/, Sampler& sampler, uint32_t sampler_dimension,
                  float2       importance_uv, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                  Sample_from& sample) const noexcept {
    float const r = sampler.generate_sample_1D(sampler_dimension);
    auto const  s = distributions_[part].sample(r);

    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r0, sv, tc);
    float3 const ws = transformation.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(transformation.rotation, sn);

    auto const [x, y] = orthonormal_basis(wn);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, wn);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / ((1.f * Pi) * area);

    return true;
}

float Mesh::pdf(Ray const& ray, shape::Intersection const&      intersection,
                Transformation const& /*transformation*/, float area, bool two_sided,
                bool /*total_sphere*/) const noexcept {
    float c = -dot(intersection.geo_n, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t * ray.max_t;
    return sl / (c * area);
}

float Mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float /*area*/) const noexcept {
    return 0.f;
}

bool Mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const noexcept {
    return 0.f;
}

float Mesh::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Mesh::area(uint32_t part, float3 const& scale) const noexcept {
    // HACK: This only really works for uniform scales!
    return distributions_[part].distribution.integral() * (scale[0] * scale[1]);
}

float Mesh::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    // HACK: This only really works for uniform scales!
    return 1.f;
}

bool Mesh::is_complex() const noexcept {
    return true;
}

bool Mesh::is_analytical() const noexcept {
    return false;
}

void Mesh::prepare_sampling(uint32_t part) noexcept {
    if (distributions_[part].empty()) {
        auto& d = distributions_[part];

        d.init(part, tree_);

        float3 center(0.f);

        float const n = 1.f / float(d.num_triangles);

        for (uint32_t i = 0, len = d.num_triangles; i < len; ++i) {
            uint32_t const t = d.triangle_mapping[i];

            center += n * tree_.triangle_center(t);
        }

        d.center = center;
    }
}

float3 Mesh::center(uint32_t part) const noexcept {
    return distributions_[part].center;
}

size_t Mesh::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (uint32_t i = 0, len = tree_.num_parts(); i < len; ++i) {
        num_bytes += distributions_[i].num_bytes();
    }

    return sizeof(*this) + tree_.num_bytes() + num_bytes;
}

Mesh::Distribution::~Distribution() {
    memory::free_aligned(triangle_mapping);
}

void Mesh::Distribution::init(uint32_t part, const Tree& tree) noexcept {
    uint32_t const num = tree.num_triangles(part);

    memory::Buffer<float> areas(num);

    num_triangles = num;

    triangle_mapping = memory::allocate_aligned<uint32_t>(num);

    for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_material_index(t) == part) {
            areas[mt] = tree.triangle_area(t);

            triangle_mapping[mt] = t;

            ++mt;
        }
    }

    distribution.init(areas, num_triangles);
}

bool Mesh::Distribution::empty() const noexcept {
    return nullptr == triangle_mapping;
}

Mesh::Distribution::Distribution_1D::Discrete Mesh::Distribution::sample(float r) const noexcept {
    auto const result = distribution.sample_discrete(r);
    return {triangle_mapping[result.offset], result.pdf};
}

size_t Mesh::Distribution::num_bytes() const noexcept {
    return sizeof(*this) + num_triangles * sizeof(uint32_t) + distribution.num_bytes();
}

}  // namespace scene::shape::triangle
