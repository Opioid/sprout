#include "triangle_mesh.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling.inl"
#include "base/math/simd_matrix.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "triangle_intersection.hpp"

namespace scene::shape::triangle {

Mesh::Mesh() noexcept : distributions_(nullptr) {}

Mesh::~Mesh() noexcept {
    memory::destroy_aligned(distributions_, tree_.num_parts());
}

bool Mesh::init() noexcept {
    aabb_       = tree_.aabb();
    inv_extent_ = 1.f / aabb_.extent();

    distributions_ = memory::construct_aligned<Distribution>(tree_.num_parts());

    return 0 != tree_.num_parts();
}

Tree& Mesh::tree() noexcept {
    return tree_;
}

uint32_t Mesh::num_parts() const noexcept {
    return tree_.num_parts();
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

    Matrix4 world_to_object = load_float4x4(transformation.world_to_object);

    Vector ray_origin = simd::load_float4(ray.origin.v);
    ray_origin        = transform_point(world_to_object, ray_origin);

    Vector ray_direction = simd::load_float4(ray.direction.v);
    ray_direction        = transform_vector(world_to_object, ray_direction);

    Vector ray_inv_direction = math::reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    math::sign(ray_inv_direction, ray_signs);

    Vector ray_min_t = simd::load_float(&ray.min_t);
    Vector ray_max_t = simd::load_float(&ray.max_t);

    Intersection pi;
    if (tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t, ray_max_t,
                        ray_signs, node_stack, pi)) {
        float const tray_max_t = simd::get_x(ray_max_t);
        ray.max_t              = tray_max_t;

        Vector p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Matrix4 object_to_world = load_float4x4(transformation.object_to_world);

        Vector p_w = transform_point(object_to_world, p);

        Vector n;
        Vector t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Vector geo_n = tree_.triangle_normal_v(pi.index);

        Vector bitangent_sign = simd::set_float4(tree_.triangle_bitangent_sign(pi.index));

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        Matrix3 rotation = load_float3x3(transformation.rotation);

        Vector geo_n_w = transform_vector(rotation, geo_n);
        Vector n_w     = transform_vector(rotation, n);
        Vector t_w     = transform_vector(rotation, t);
        Vector b_w     = mul(bitangent_sign, cross3(n_w, t_w));

        simd::store_float4(intersection.p.v, p_w);
        simd::store_float4(intersection.t.v, t_w);
        simd::store_float4(intersection.b.v, b_w);
        simd::store_float4(intersection.n.v, n_w);
        simd::store_float4(intersection.geo_n.v, geo_n_w);

        intersection.uv   = uv;
        intersection.part = material_index;

        return true;
    }

    return false;
}

bool Mesh::intersect_fast(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                          shape::Intersection& intersection) const noexcept {
    Matrix4 world_to_object = math::load_float4x4(transformation.world_to_object);

    Vector ray_origin = simd::load_float4(ray.origin.v);
    ray_origin        = transform_point(world_to_object, ray_origin);

    Vector ray_direction = simd::load_float4(ray.direction.v);
    ray_direction        = transform_vector(world_to_object, ray_direction);

    Vector ray_inv_direction = math::reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    math::sign(ray_inv_direction, ray_signs);

    Vector ray_min_t = simd::load_float(&ray.min_t);
    Vector ray_max_t = simd::load_float(&ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        float tray_max_t = simd::get_x(ray_max_t);
        ray.max_t        = tray_max_t;

        Vector p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Matrix4 object_to_world = load_float4x4(transformation.object_to_world);

        Vector p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Vector geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        Matrix3 rotation = math::load_float3x3(transformation.rotation);
        Vector  geo_n_w  = transform_vector(rotation, geo_n);

        simd::store_float4(intersection.p.v, p_w);
        simd::store_float4(intersection.geo_n.v, geo_n_w);
        intersection.uv   = uv;
        intersection.part = material_index;

        return true;
    }

    return false;
}

bool Mesh::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     Normals& normals) const noexcept {
    const Matrix4 world_to_object = math::load_float4x4(transformation.world_to_object);

    Vector ray_origin = simd::load_float4(ray.origin.v);
    ray_origin        = transform_point(world_to_object, ray_origin);

    Vector ray_direction = simd::load_float4(ray.direction.v);
    ray_direction        = transform_vector(world_to_object, ray_direction);

    Vector const ray_inv_direction = math::reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    math::sign(ray_inv_direction, ray_signs);

    Vector const ray_min_t = simd::load_float(&ray.min_t);
    Vector       ray_max_t = simd::load_float(&ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        ray.max_t = simd::get_x(ray_max_t);

        Vector n = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);

        Vector geo_n = tree_.triangle_normal_v(pi.index);

        Matrix3 rotation = load_float3x3(transformation.rotation);

        Vector geo_n_w = transform_vector(rotation, geo_n);
        Vector n_w     = transform_vector(rotation, n);

        simd::store_float4(normals.geo_n.v, geo_n_w);
        simd::store_float4(normals.n.v, n_w);

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

    Matrix4 world_to_object = math::load_float4x4(transformation.world_to_object);

    Vector ray_origin = simd::load_float4(ray.origin.v);
    ray_origin        = transform_point(world_to_object, ray_origin);

    Vector ray_direction = simd::load_float4(ray.direction.v);
    ray_direction        = transform_vector(world_to_object, ray_direction);

    Vector ray_inv_direction = math::reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    math::sign(ray_inv_direction, ray_signs);

    Vector ray_min_t = simd::load_float(&ray.min_t);
    Vector ray_max_t = simd::load_float(&ray.max_t);

    return tree_.intersect_p(ray_origin, ray_direction, ray_inv_direction, ray_min_t, ray_max_t,
                             ray_signs, node_stack);
}

// bool Mesh::intersect_p(FVector ray_origin, FVector ray_direction,
//					   FVector ray_min_t, FVector ray_max_t,
//					   Transformation const& transformation,
//					   Node_stack& node_stack) const noexcept {
//	Matrix4 world_to_object = math::load_float4x4(transformation.world_to_object);
//	Vector tray_origin = transform_point(world_to_object, ray_origin);
//	Vector tray_direction = transform_vector(world_to_object, ray_direction);

//	Vector tray_inv_direction = math::reciprocal3(tray_direction);
//	alignas(16) uint32_t ray_signs[4];
//	math::sign(tray_inv_direction, ray_signs);

//	return tree_.intersect_p(tray_origin, tray_direction, tray_inv_direction,
//							 ray_min_t, ray_max_t, ray_signs,
// node_stack);
//}

float Mesh::opacity(Ray const& ray, Transformation const& transformation, Materials materials,
                    Filter filter, Worker const& worker) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.opacity(tray, ray.time, materials, filter, worker);
}

Visibility Mesh::thin_absorption(Ray const& ray, Transformation const& transformation,
                                 Materials materials, Filter filter, Worker const& worker,
                                 float3& ta) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.absorption(tray, ray.time, materials, filter, worker, ta);
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
    float3 const v = transform_point(transformation.object_to_world, sv);

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

    if (c <= 0.f) {
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
                  float2 const& importance_uv, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                  Sample_from&  sample) const noexcept {
    float const r = sampler.generate_sample_1D(sampler_dimension);
    auto const  s = distributions_[part].sample(r);

    float2 const r0 = sampler.generate_sample_2D(sampler_dimension);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r0, sv, tc);
    float3 const ws = transform_point(transformation.object_to_world, sv);

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
                  float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                  uint32_t /*sampler_dimension*/, float2 const& /*importance_uv*/,
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
        distributions_[part].init(part, tree_);
    }
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

    float* areas = memory::allocate_aligned<float>(num);

    num_triangles = num;

    triangle_mapping = memory::allocate_aligned<uint32_t>(num);

    for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_material_index(t) == part) {
            areas[mt]            = tree.triangle_area(t);
            triangle_mapping[mt] = t;
            ++mt;
        }
    }

    distribution.init(areas, num_triangles);

    memory::free_aligned(areas);
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
