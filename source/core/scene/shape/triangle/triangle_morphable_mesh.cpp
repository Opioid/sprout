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
#include "triangle_primitive.hpp"

#ifdef SU_DEBUG
#include "scene/shape/shape_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::shape::triangle {

Morphable_mesh::Morphable_mesh(Morph_target_collection&& collection, uint32_t num_parts)
    : Shape(Properties(Property::Complex, Property::Finite)),
      collection_(std::move(collection)),
      vertices_(new Vertex[collection_.num_vertices()]) {
    tree_.allocate_parts(num_parts);
}

Morphable_mesh::~Morphable_mesh() {
    delete[] vertices_;
}

AABB Morphable_mesh::aabb() const {
    return tree_.aabb();
}

uint32_t Morphable_mesh::num_parts() const {
    return tree_.num_parts();
}

bool Morphable_mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                               Interpolation ipo, shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simdf const ray_origin    = transform_point(world_to_object, Simdf(ray.origin));
    Simdf const ray_direction = transform_vector(world_to_object, Simdf(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simdf p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(trafo.object_to_world());

        Simdf p_w = transform_point(object_to_world, p);

        Simdf geo_n = tree_.triangle_normal(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simdf geo_n_w = transform_vector(rotation, geo_n);

        isec.p     = float3(p_w);
        isec.geo_n = float3(geo_n_w);
        isec.part  = tree_.triangle_part(pi.index);
        ;
        isec.primitive = pi.index;

        if (Interpolation::All == ipo) {
            Simdf  n;
            Simdf  t;
            float2 uv;
            tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

            Simdf const bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

            Simdf const n_w = transform_vector(rotation, n);
            Simdf const t_w = transform_vector(rotation, t);
            Simdf const b_w = bitangent_sign * cross3(n_w, t_w);

            isec.t = float3(t_w);
            isec.b = float3(b_w);
            isec.n = float3(n_w);

            isec.uv = uv;
        } else if (Interpolation::No_tangent_space == ipo) {
            float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

            isec.uv = uv;
        } else {
            Simdf const n   = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);
            Simdf const n_w = transform_vector(rotation, n);

            isec.n = float3(n_w);
        }

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Morphable_mesh::intersect_p(Ray const& ray, Transformation const& trafo,
                                 Node_stack& nodes) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simdf const ray_origin    = transform_point(world_to_object, Simdf(ray.origin));
    Simdf const ray_direction = transform_vector(world_to_object, Simdf(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    return tree_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes);
}

bool Morphable_mesh::visibility(Ray const& ray, Transformation const& trafo, uint32_t entity,
                                Filter filter, Worker& worker, float3& v) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simdf const ray_origin    = transform_point(world_to_object, Simdf(ray.origin));
    Simdf const ray_direction = transform_vector(world_to_object, Simdf(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    return tree_.visibility(ray_origin, ray_direction, ray_min_t, ray_max_t, entity, filter, worker,
                            v);
}

bool Morphable_mesh::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p /*p*/, float3_p /*n*/,
                            Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                            bool /*total_sphere*/, sampler::Sampler& /*sampler*/, RNG& /*rng*/,
                            uint32_t /*sampler_d*/, Sample_to& /*sample*/) const {
    return false;
}

bool Morphable_mesh::sample(uint32_t /*part*/, uint32_t /*variant*/,
                            Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                            sampler::Sampler& /*sampler*/, RNG& /*rng*/, uint32_t /*sampler_d*/,
                            float2 /*importance_uv*/, AABB const& /*bounds*/,
                            Sample_from& /*sample*/) const {
    return false;
}

float Morphable_mesh::pdf(uint32_t /*variant*/, Ray const& /*ray*/, float3_p /*n*/,
                          shape::Intersection const& /*isec*/, Transformation const& /*trafo*/,
                          float /*area*/, bool /*two_sided*/, bool /*total_sphere*/) const {
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

void Morphable_mesh::morph(Morphing const& a, Threads& threads) {
    collection_.morph(a, threads, vertices_);

    Vertex_stream_interleaved vertices(collection_.num_vertices(), vertices_);

    bvh::Builder_SAH builder(16, 64, 4);
    builder.build(tree_, uint32_t(collection_.triangles().size()), collection_.triangles().data(),
                  vertices, threads);
}

}  // namespace scene::shape::triangle
