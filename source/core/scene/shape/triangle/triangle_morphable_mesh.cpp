#include "triangle_morphable_mesh.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
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

Morphable_mesh::Morphable_mesh(Morph_target_collection* collection, uint32_t num_parts)
    : Shape(Properties(Property::Complex, Property::Finite)),
      collection_(collection),
      vertices_(memory::allocate_aligned<Vertex>(collection->num_vertices())) {
    tree_.allocate_parts(num_parts);
}

Morphable_mesh::~Morphable_mesh() {
    memory::free_aligned(vertices_);
    delete collection_;
}

float3 Morphable_mesh::object_to_texture_point(float3 const& p) const {
    return (p - tree_.aabb().bounds[0]) / tree_.aabb().extent();
}

float3 Morphable_mesh::object_to_texture_vector(float3 const& v) const {
    return v / tree_.aabb().extent();
}

AABB Morphable_mesh::transformed_aabb(float4x4 const& m) const {
    return tree_.aabb().transform(m);
}

uint32_t Morphable_mesh::num_parts() const {
    return tree_.num_parts();
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
                               Node_stack& node_stack, shape::Intersection& intersection) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar ray_min_t(ray.min_t());
    scalar ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t part = tree_.triangle_part(pi.index);

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
        intersection.part = part;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_nsf(Ray& ray, Transformation const& transformation,
                                   Node_stack&          node_stack,
                                   shape::Intersection& intersection) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t const part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);

        intersection.p     = float3(p_w);
        intersection.geo_n = float3(geo_n_w);
        intersection.uv    = uv;
        intersection.part  = part;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& transformation,
                               Node_stack& node_stack, Normals& normals) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

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

bool Morphable_mesh::intersect_p(Ray const& ray, Transformation const& transformation,
                                 Node_stack& node_stack) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    return tree_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack);
}

float Morphable_mesh::visibility(Ray const& ray, Transformation const& transformation,
                                 uint32_t entity, Filter filter, Worker& worker) const {
    math::ray tray(transformation.world_to_object_point(ray.origin),
                   transformation.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.visibility(tray, ray.time, entity, filter, worker);
}

bool Morphable_mesh::thin_absorption(Ray const& ray, Transformation const& transformation,
                                     uint32_t entity, Filter filter, Worker& worker,
                                     float3& ta) const {
    math::ray tray(transformation.world_to_object_point(ray.origin),
                   transformation.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.absorption(tray, ray.time, entity, filter, worker, ta);
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, Transformation const& /*transformation*/,
                            float /*area*/, bool /*two_sided*/, sampler::Sampler& /*sampler*/,
                            uint32_t /*sampler_dimension*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Morphable_mesh::pdf(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                          Transformation const& /*transformation*/, float /*area*/,
                          bool /*two_sided*/, bool /*total_sphere*/) const {
    return 0.f;
}

float Morphable_mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                                 Transformation const& /*transformation*/, float /*volume*/) const {
    return 0.f;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                            Transformation const& /*transformation*/, float /*volume*/,
                            Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float2 /*uv*/,
                            Transformation const& /*transformation*/, float /*area*/,
                            bool /*two_sided*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                            Sample_from& /*sample*/) const {
    return false;
}

float Morphable_mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                             Transformation const& /*transformation*/, float /*area*/,
                             bool /*two_sided*/) const {
    return 0.f;
}

float Morphable_mesh::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Morphable_mesh::area(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 1.f;
}

float Morphable_mesh::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 1.f;
}

Shape::Differential_surface Morphable_mesh::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

void Morphable_mesh::prepare_sampling(uint32_t /*part*/) {}

Morphable* Morphable_mesh::morphable_shape() {
    return this;
}

void Morphable_mesh::morph(uint32_t a, uint32_t b, float weight, thread::Pool& threads) {
    collection_->morph(a, b, weight, threads, vertices_);

    Vertex_stream_interleaved vertices(collection_->num_vertices(), vertices_);

    bvh::Builder_SAH builder(16, 64);
    builder.build(tree_, uint32_t(collection_->triangles().size()), collection_->triangles().data(),
                  vertices, 4, threads);
}

}  // namespace scene::shape::triangle
