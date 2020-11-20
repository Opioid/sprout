#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "scene/shape/shape.hpp"
#include "scene/light/light_tree.hpp"

namespace scene::shape::triangle {

class alignas(64) Mesh final : public Shape {
  public:
    Mesh();

    ~Mesh() final;

    bvh::Tree& tree();

    void allocate_parts(uint32_t num_parts);

    void set_material_for_part(uint32_t part, uint32_t material);

    float3 object_to_texture_point(float3_p p) const final;

    float3 object_to_texture_vector(float3_p v) const final;

    AABB transformed_aabb(float4x4 const& m) const final;

    AABB transformed_part_aabb(uint32_t part, float4x4 const& m) const final;

    uint32_t num_parts() const final;

    uint32_t num_materials() const final;

    uint32_t part_id_to_material_id(uint32_t part) const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                   shape::Intersection& isec) const final;

    bool intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                       shape::Intersection& isec) const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                   Normals& normals) const final;

    bool intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const final;

    float visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                     Worker& worker) const final;

    bool thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                         Filter filter, Worker& worker, float3& ta) const final;

    bool sample(uint32_t part, float3_p p, Transformation const& trafo, float area, bool two_sided,
                Sampler& sampler, RNG& rng, uint32_t sampler_d, Sample_to& sample) const final;

    bool sample(uint32_t part, Transformation const& trafo, float area, bool two_sided,
                Sampler& sampler, RNG& rng, uint32_t sampler_d, float2 importance_uv,
                AABB const& bounds, Sample_from& sample) const final;

    float pdf(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
              float area, bool two_sided, bool total_sphere) const final;

    float pdf_volume(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                     float volume) const final;

    bool sample(uint32_t part, float3_p p, float2 uv, Transformation const& trafo, float area,
                bool two_sided, Sample_to& sample) const final;

    bool sample(uint32_t part, float3_p p, float3_p uvw, Transformation const& trafo, float volume,
                Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area, bool two_sided,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                 float area, bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3_p scale) const final;

    float volume(uint32_t part, float3_p scale) const final;

    Differential_surface differential_surface(uint32_t primitive) const final;

    void prepare_sampling(uint32_t part) final;

    float4 cone(uint32_t part) const final;

  private:
    bvh::Tree tree_;

    struct Part {
        using Distribution_1D = math::Distribution_1D;

        ~Part();

        void init(uint32_t part, bvh::Tree const& tree);

        Distribution_1D::Discrete sample(float r) const;

        uint32_t material;

        uint32_t num_triangles = 0xFFFFFFFF;

        uint32_t* triangle_mapping = nullptr;

        AABB* aabbs = nullptr;

        float4* cones = nullptr;

        Distribution_1D distribution;

        light::Primitive_tree light_tree;

        AABB aabb;

        float4 cone;
    };

    Part* parts_;
};

}  // namespace scene::shape::triangle

#endif
