#include "triangle_morphable_mesh.hpp"
#include "bvh/triangle_bvh_builder_sah.inl"
#include "bvh/triangle_bvh_indexed_data.inl"
#include "triangle_intersection.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive_mt.hpp"
//#include "bvh/triangle_bvh_data_interleaved.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::shape::triangle {

Morphable_mesh::Morphable_mesh(std::shared_ptr<Morph_target_collection> collection,
                               uint32_t                                 num_parts) noexcept
    : collection_(collection) {
    tree_.allocate_parts(num_parts);
    vertices_.resize(collection_->vertices(0).size());
}

void Morphable_mesh::init() noexcept {
    aabb_ = tree_.aabb();
}

uint32_t Morphable_mesh::num_parts() const noexcept {
    return tree_.num_parts();
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
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

        float epsilon = 3e-3f * tray.max_t;

        float3 p_w = ray.point(tray.max_t);

        float3 n;
        float3 t;
        float2 uv;
        //	tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        float3   geo_n          = tree_.triangle_normal(pi.index);
        float    bitangent_sign = tree_.triangle_bitangent_sign(pi.index);
        uint32_t material_index = tree_.triangle_material_index(pi.index);

        float3 geo_n_w = transform_vector(transformation.rotation, geo_n);
        float3 n_w     = transform_vector(transformation.rotation, n);
        float3 t_w     = transform_vector(transformation.rotation, t);
        float3 b_w     = bitangent_sign * cross(n_w, t_w);

        intersection.p       = p_w;
        intersection.t       = t_w;
        intersection.b       = b_w;
        intersection.n       = n_w;
        intersection.geo_n   = geo_n_w;
        intersection.uv      = uv;
        intersection.epsilon = epsilon;
        intersection.part    = material_index;

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

        float epsilon = 3e-3f * tray.max_t;

        float3 p_w = ray.point(tray.max_t);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        float3 geo_n = tree_.triangle_normal(pi.index);

        uint32_t material_index = tree_.triangle_material_index(pi.index);

        float3 geo_n_w = transform_vector(transformation.rotation, geo_n);

        intersection.p       = p_w;
        intersection.geo_n   = geo_n_w;
        intersection.uv      = uv;
        intersection.epsilon = epsilon;
        intersection.part    = material_index;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
                               Node_stack& node_stack, float& epsilon) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    if (tree_.intersect(tray, node_stack)) {
        ray.max_t = tray.max_t;
        epsilon   = 3e-3f * tray.max_t;
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
                              Materials const& materials, Filter filter, Worker const& worker) const
    noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.opacity(tray, ray.time, materials, filter, worker);
}

float3 Morphable_mesh::thin_absorption(Ray const& ray, Transformation const& transformation,
                                       Materials const& materials, Filter filter,
                                       Worker const& worker) const noexcept {
    math::ray tray;
    tray.origin = transform_point(transformation.world_to_object, ray.origin);
    tray.set_direction(transform_vector(transformation.world_to_object, ray.direction));
    tray.min_t = ray.min_t;
    tray.max_t = ray.max_t;

    return tree_.absorption(tray, ray.time, materials, filter, worker);
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
                            uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                            Node_stack& /*node_stack*/, Sample_from& /*sample*/) const noexcept {
    return false;
}

float Morphable_mesh::pdf(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
                          Transformation const& /*transformation*/, float /*area*/,
                          bool /*two_sided*/, bool /*total_sphere*/) const noexcept {
    return 0.f;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                            Sample_from& /*sample*/) const noexcept {
    return false;
}

float Morphable_mesh::pdf_uv(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
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

    bvh::Builder_SAH builder(16, 64);
    builder.build(tree_, collection_->triangles(), vertices_, 4, pool);

    init();
}

size_t Morphable_mesh::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape::triangle
