#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "scene/shape/shape.hpp"
#include "triangle_mesh_bvh.hpp"

namespace scene::shape::triangle {

class alignas(64) Mesh : public Shape {
  public:
    Mesh() noexcept;

    ~Mesh() noexcept override;

    Tree& tree() noexcept;

    void allocate_parts(uint32_t num_parts) noexcept;

    void set_material_for_part(uint32_t part, uint32_t material) noexcept;

    float3 object_to_texture_point(float3 const& p) const noexcept final;

    float3 object_to_texture_vector(float3 const& v) const noexcept final;

    AABB transformed_aabb(float4x4 const& m) const noexcept final;

    uint32_t num_parts() const noexcept final;

    uint32_t num_materials() const noexcept final;

    uint32_t part_id_to_material_id(uint32_t part) const noexcept final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   shape::Intersection& intersection) const noexcept final;

    bool intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                       shape::Intersection& intersection) const noexcept final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   Normals& normals) const noexcept final;

    bool intersect_p(Ray const& ray, Transformation const& transformation,
                     Node_stack& node_stack) const noexcept final;

    float opacity(Ray const& ray, Transformation const& transformation, uint32_t entity,
                  Filter filter, Worker const& worker) const noexcept final;

    bool thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                         Filter filter, Worker const& worker, float3& ta) const noexcept final;

    bool sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                Node_stack& node_stack, Sample_to& sample) const noexcept final;

    bool sample(uint32_t part, Transformation const& transformation, float area, bool two_sided,
                Sampler& sampler, uint32_t sampler_dimension, float2 importance_uv,
                AABB const& bounds, Node_stack& node_stack, Sample_from& sample) const
        noexcept final;

    float pdf(Ray const& ray, shape::Intersection const& intersection,
              Transformation const& transformation, float area, bool two_sided,
              bool total_sphere) const noexcept final;

    float pdf_volume(Ray const& ray, shape::Intersection const& intersection,
                     Transformation const& transformation, float volume) const noexcept final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& transformation,
                float area, bool two_sided, Sample_to& sample) const noexcept final;

    bool sample(uint32_t part, float3 const& p, float3 const& uvw,
                Transformation const& transformation, float volume, Sample_to& sample) const
        noexcept final;

    bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                bool two_sided, float2 importance_uv, AABB const& bounds, Sample_from& sample) const
        noexcept final;

    float pdf_uv(Ray const& ray, shape::Intersection const& intersection,
                 Transformation const& transformation, float area, bool two_sided) const
        noexcept final;

    float uv_weight(float2 uv) const noexcept final;

    float area(uint32_t part, float3 const& scale) const noexcept final;

    float volume(uint32_t part, float3 const& scale) const noexcept final;

    bool is_complex() const noexcept final;

    bool is_analytical() const noexcept final;

    void prepare_sampling(uint32_t part) noexcept final;

    float3 center(uint32_t part) const noexcept final;

    size_t num_bytes() const noexcept final;

  private:
    Tree tree_;

    struct Distribution {
        using Distribution_1D = math::Distribution_implicit_pdf_lut_lin_1D;

        ~Distribution();

        void init(uint32_t part, const Tree& tree) noexcept;

        bool empty() const noexcept;

        Distribution_1D::Discrete sample(float r) const noexcept;

        size_t num_bytes() const noexcept;

        uint32_t  num_triangles    = 0;
        uint32_t* triangle_mapping = nullptr;

        Distribution_1D distribution;

        float3 center;
    };

    Distribution* distributions_;

    uint32_t* part_materials_;
};

}  // namespace scene::shape::triangle

#endif
