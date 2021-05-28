#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP

#include "base/math/distribution_1d.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "scene/light/light_tree.hpp"
#include "scene/shape/shape.hpp"

#include <vector>

namespace scene::shape::triangle {

struct Part {
    using Distribution_1D = math::Distribution_1D;
    using Material        = material::Material;

    struct Variant {
        Variant();

        Variant(Variant&& other);

        bool matches(uint32_t m, bool emission_map, bool two_sided, Scene const& scene) const;

        Distribution_1D distribution;

        light::Primitive_tree light_tree;

        AABB aabb;

        float4 cone;

        uint32_t material;

        bool two_sided_;
    };

    ~Part();

    uint32_t init(uint32_t part, uint32_t material, bvh::Tree const& tree,
                  light::Tree_builder& builder, Worker const& worker, Threads& threads);

    struct Discrete {
        uint32_t global;
        uint32_t local;
        float    pdf;
    };

    Discrete sample(uint32_t variant, float3_p p, float3_p n, bool total_sphere, float r) const;

    float pdf(uint32_t variant, float3_p p, float3_p n, bool total_sphere, uint32_t id) const;

    Discrete sample(uint32_t variant, float r) const;

    AABB const& aabb(uint32_t variant) const;

    float power(uint32_t variant) const;

    float4_p cone(uint32_t variant) const;

    AABB const& light_aabb(uint32_t light) const;

    float4_p light_cone(uint32_t light) const;

    bool light_two_sided(uint32_t variant, uint32_t light) const;

    float light_power(uint32_t variant, uint32_t light) const;

    uint32_t material_;

    uint32_t num_triangles_ = 0;

    uint32_t* triangle_mapping_ = nullptr;

    AABB* aabbs_ = nullptr;

    float4* cones_ = nullptr;

    std::vector<Variant> variants_;

    float area_;
};

class alignas(64) Mesh final : public Shape {
  public:
    Mesh();

    ~Mesh() final;

    bvh::Tree& tree();

    void allocate_parts(uint32_t num_parts);

    void set_material_for_part(uint32_t part, uint32_t material);

    AABB aabb() const final;

    AABB part_aabb(uint32_t part, uint32_t variant) const final;

    uint32_t num_parts() const final;

    uint32_t num_materials() const final;

    uint32_t part_id_to_material_id(uint32_t part) const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes, Interpolation ipo,
                   shape::Intersection& isec) const final;

    bool intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const final;

    bool visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                    Worker& worker, float3& ta) const final;

    bool sample(uint32_t part, uint32_t variant, float3_p p, float3_p n,
                Transformation const& trafo, float area, bool two_sided, bool total_sphere,
                Sampler& sampler, RNG& rng, uint32_t sampler_d, Sample_to& sample) const final;

    bool sample(uint32_t part, uint32_t variant, Transformation const& trafo, float area,
                bool two_sided, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf(uint32_t variant, Ray const& ray, float3_p n, shape::Intersection const& isec,
              Transformation const& trafo, float area, bool two_sided,
              bool total_sphere) const final;

    float pdf_volume(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                     float volume) const final;

    bool sample(uint32_t part, float3_p p, float2 uv, Transformation const& trafo, float area,
                bool two_sided, Sample_to& sample) const final;

    bool sample_volume(uint32_t part, float3_p p, float3_p uvw, Transformation const& trafo,
                       float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area, bool two_sided,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                 float area, bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3_p scale) const final;

    float volume(uint32_t part, float3_p scale) const final;

    Differential_surface differential_surface(uint32_t primitive) const final;

    uint32_t prepare_sampling(uint32_t part, uint32_t material, light::Tree_builder& builder,
                              Worker const& worker, Threads& threads) final;

    float4 cone(uint32_t part) const final;

  private:
    bvh::Tree tree_;

    Part* parts_;

    uint32_t* primitive_mapping_;
};

}  // namespace scene::shape::triangle

#endif
