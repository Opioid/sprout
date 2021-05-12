#include "triangle_morphable_mesh.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "bvh/triangle_bvh_builder_sah.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "triangle_intersection.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive.hpp"

namespace scene::shape::triangle {

Morphable_mesh::Morphable_mesh(Morph_target_collection* collection, uint32_t num_parts)
    : Shape(Properties(Property::Complex, Property::Finite)),
      collection_(collection),
      vertices_(new Vertex[collection->num_vertices()]) {
    tree_.allocate_parts(num_parts);
}

Morphable_mesh::~Morphable_mesh() {
    delete[] vertices_;
    delete collection_;
}

AABB Morphable_mesh::aabb() const {
    return tree_.aabb();
}

uint32_t Morphable_mesh::num_parts() const {
    return tree_.num_parts();
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                               shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar ray_min_t(ray.min_t());
    scalar ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f object_to_world(trafo.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);
        Simd3f t_w     = transform_vector(rotation, t);
        Simd3f b_w     = bitangent_sign * cross(n_w, t_w);

        isec.p     = float3(p_w);
        isec.t     = float3(t_w);
        isec.b     = float3(b_w);
        isec.n     = float3(n_w);
        isec.geo_n = float3(geo_n_w);

        isec.uv   = uv;
        isec.part = part;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                                   shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(trafo.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t const part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);

        isec.p     = float3(p_w);
        isec.geo_n = float3(geo_n_w);
        isec.uv    = uv;
        isec.part  = part;

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                               Normals& normals) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f n = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);

        normals.geo_n = float3(geo_n_w);
        normals.n     = float3(n_w);

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_p(Ray const& ray, Transformation const& trafo,
                                 Node_stack& nodes) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    return tree_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes);
}

float Morphable_mesh::visibility(Ray const& ray, Transformation const& trafo, uint32_t entity,
                                 Filter filter, Worker& worker) const {
    math::ray tray(trafo.world_to_object_point(ray.origin),
                   trafo.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.visibility(tray, entity, filter, worker);
}

bool Morphable_mesh::thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                                     Filter filter, Worker& worker, float3& ta) const {
    math::ray tray(trafo.world_to_object_point(ray.origin),
                   trafo.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.absorption(tray, entity, filter, worker, ta);
}

bool Morphable_mesh::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p /*p*/, float3_p /*n*/,
                            Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                            bool /*total_sphere*/, sampler::Sampler& /*sampler*/, RNG& /*rng*/,
                            uint32_t /*sampler_d*/, Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, Transformation const& /*trafo*/, float /*area*/,
                            bool /*two_sided*/, sampler::Sampler& /*sampler*/, RNG& /*rng*/,
                            uint32_t /*sampler_d*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Morphable_mesh::pdf(Ray const& /*ray*/, float3_p /*n*/, shape::Intersection const& /*isec*/,
                          Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                          bool /*total_sphere*/) const {
    return 0.f;
}

float Morphable_mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*isec*/,
                                 Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/,
                            Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                            Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                                   Transformation const& /*trafo*/, float /*volume*/,
                                   Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/,
                            float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                            AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Morphable_mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*isec*/,
                             Transformation const& /*trafo*/, float /*area*/,
                             bool /*two_sided*/) const {
    return 0.f;
}

float Morphable_mesh::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Morphable_mesh::area(uint32_t /*part*/, float3_p /*scale*/) const {
    return 1.f;
}

float Morphable_mesh::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 1.f;
}

Shape::Differential_surface Morphable_mesh::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

Morphable* Morphable_mesh::morphable_shape() {
    return this;
}

void Morphable_mesh::morph(uint32_t a, uint32_t b, float weight, Threads& threads) {
    collection_->morph(a, b, weight, threads, vertices_);

    Vertex_stream_interleaved vertices(collection_->num_vertices(), vertices_);

    bvh::Builder_SAH builder(16, 64, 4);
    builder.build(tree_, uint32_t(collection_->triangles().size()), collection_->triangles().data(),
                  vertices, threads);
}

}  // namespace scene::shape::triangle
