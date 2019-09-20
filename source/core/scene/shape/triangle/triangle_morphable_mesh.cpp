#include "triangle_morphable_mesh.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/simd_matrix.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "bvh/triangle_bvh_builder_sah.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "triangle_intersection.hpp"
#include "triangle_morph_target_collection.hpp"

namespace scene::shape::triangle {

Morphable_mesh::Morphable_mesh(Morph_target_collection* collection, uint32_t num_parts) noexcept
    : collection_(collection),
      vertices_(memory::allocate_aligned<Vertex>(collection->num_vertices())) {
    tree_.allocate_parts(num_parts);
}

Morphable_mesh::~Morphable_mesh() {
    memory::free_aligned(vertices_);
    delete collection_;
}

float3 Morphable_mesh::object_to_texture_point(float3 const& p) const noexcept {
    return (p - tree_.aabb().bounds[0]) / tree_.aabb().extent();
}

float3 Morphable_mesh::object_to_texture_vector(float3 const& v) const noexcept {
    return v / tree_.aabb().extent();
}

AABB Morphable_mesh::transformed_aabb(float4x4 const& m, math::Transformation const& /*t*/) const
    noexcept {
    return tree_.aabb().transform(m);
}

AABB Morphable_mesh::transformed_aabb(math::Transformation const& t) const noexcept {
    return transformed_aabb(float4x4(t), t);
}

uint32_t Morphable_mesh::num_parts() const noexcept {
    return tree_.num_parts();
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
                               Node_stack& node_stack, shape::Intersection& intersection) const
    noexcept {
    Matrix4 world_to_object = load_float4x4(transformation.world_to_object);

    Simd3f ray_origin(ray.origin.v);
    ray_origin = transform_point(world_to_object, ray_origin);

    Simd3f ray_direction(ray.direction.v);
    ray_direction = transform_vector(world_to_object, ray_direction);

    Simd3f ray_inv_direction = reciprocal(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    math::sign(ray_inv_direction, ray_signs);

    Simd3f ray_min_t = Simd3f::create_scalar(ray.min_t);
    Simd3f ray_max_t = Simd3f::create_scalar(ray.max_t);

    if (Intersection pi; tree_.intersect(ray_origin, ray_direction, ray_inv_direction, ray_min_t,
                                         ray_max_t, ray_signs, node_stack, pi)) {
        float const tray_max_t = ray_max_t.x();
        ray.max_t              = tray_max_t;

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Matrix4 object_to_world = load_float4x4(transformation.object_to_world);

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        Matrix3 rotation = load_float3x3(transformation.rotation);

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

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_fast(Ray& ray, Transformation const& transformation,
                                    Node_stack& node_stack, shape::Intersection& intersection) const
    noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    Intersection pi;
    if (tree_.intersect(tray, node_stack, pi)) {
        ray.max_t = tray.max_t;

        float3 p_w = ray.point(tray.max_t);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u.v, pi.v.v, pi.index);

        float3 geo_n = tree_.triangle_normal(pi.index);

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        float3 geo_n_w = transform_vector(transformation.rotation, geo_n);

        intersection.p     = p_w;
        intersection.geo_n = geo_n_w;
        intersection.uv    = uv;
        intersection.part  = material_index;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
                               Node_stack& node_stack, Normals& /*normals*/) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    if (tree_.intersect(tray, node_stack)) {
        ray.max_t = tray.max_t;
        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_p(Ray const& ray, Transformation const& transformation,
                                 Node_stack& node_stack) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.intersect_p(tray, node_stack);
}

float Morphable_mesh::opacity(Ray const& ray, Transformation const& transformation,
                              Materials materials, Filter filter, Worker const& worker) const
    noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.opacity(tray, ray.time, materials, filter, worker);
}

bool Morphable_mesh::thin_absorption(Ray const& ray, Transformation const& transformation,
                                     Materials materials, Filter filter, Worker const& worker,
                                     float3& ta) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.absorption(tray, ray.time, materials, filter, worker, ta);
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*n*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, Node_stack& /*node_stack*/,
                            Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, Node_stack& /*node_stack*/,
                            Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, Transformation const& /*transformation*/,
                            float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                            Sample_from& /*sample*/) const noexcept {
    return false;
}

float Morphable_mesh::pdf(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                          Transformation const& /*transformation*/, float /*area*/,
                          bool /*two_sided*/, bool /*total_sphere*/) const noexcept {
    return 0.f;
}

float Morphable_mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                                 Transformation const& /*transformation*/, float /*volume*/) const
    noexcept {
    return 0.f;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                            Transformation const& /*transformation*/, float /*volume*/,
                            Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Morphable_mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                             Transformation const& /*transformation*/, float /*area*/,
                             bool /*two_sided*/) const noexcept {
    return 0.f;
}

float Morphable_mesh::uv_weight(float2 /*uv*/) const noexcept {
    return 1.f;
}

float Morphable_mesh::area(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 1.f;
}

float Morphable_mesh::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 1.f;
}

bool Morphable_mesh::is_complex() const noexcept {
    return true;
}

bool Morphable_mesh::is_analytical() const noexcept {
    return false;
}

void Morphable_mesh::prepare_sampling(uint32_t /*part*/) noexcept {}

Morphable_shape* Morphable_mesh::morphable_shape() noexcept {
    return this;
}

void Morphable_mesh::morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) noexcept {
    collection_->morph(a, b, weight, pool, vertices_);

    Vertex_stream_interleaved vertices(collection_->num_vertices(), vertices_);

    bvh::Builder_SAH builder(16, 64);
    builder.build(tree_, uint32_t(collection_->triangles().size()), collection_->triangles().data(),
                  vertices, 4, pool);
}

size_t Morphable_mesh::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape::triangle
