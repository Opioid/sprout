#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_HPP

#include "triangle_type.hpp"
#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
// #include "bvh/triangle_bvh_data.hpp"
#include "bvh/triangle_bvh_indexed_data.hpp"
// #include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene::shape::triangle {

class Mesh : public Shape {

public:

	~Mesh();

//	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_type>>;

//	using Tree = bvh::Tree<bvh::Data<Intersection_triangle_type, Shading_triangle_type>>;

	using Tree = bvh::Tree<bvh::Indexed_data<Intersection_vertex_type, Shading_vertex_type>>;

//	using Tree = bvh::Tree<bvh::Indexed_data_interleaved<Vertex_type>>;

	bool init();

	Tree& tree();

	virtual uint32_t num_parts() const override final;

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   shape::Intersection& intersection) const override final;

	virtual bool intersect(const Transformation& transformation, const Ray& ray,
						   Node_stack& node_stack, float& min, float& max) const override final;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const override final;

//	virtual bool intersect_p(VVector ray_origin, VVector ray_direction,
//							 VVector ray_min_t, VVector ray_max_t,
//							 const Transformation& transformation,
//							 Node_stack& node_stack) const override final;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials, Sampler_filter filter,
						  const Worker& worker) const override final;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials, Sampler_filter filter,
								   const Worker& worker) const override final;

	virtual bool sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual bool sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(const Ray& ray, const shape::Intersection& intersection,
					  const Transformation& transformation,
					  float area, bool two_sided, bool total_sphere) const override final;

	virtual bool sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(const Ray& ray, const shape::Intersection& intersection,
						 const Transformation& transformation,
						 float area, bool two_sided) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, const float3& scale) const override final;

	virtual bool is_complex() const override final;

	virtual bool is_analytical() const override final;

	virtual void prepare_sampling(uint32_t part) override final;

	virtual size_t num_bytes() const override final;

private:

    Tree tree_;

	struct Distribution {
		using Distribution_1D = math::Distribution_implicit_pdf_lut_lin_1D;

		void init(uint32_t part, const Tree& tree);
		bool empty() const;

		Distribution_1D::Discrete sample(float r) const;

		size_t num_bytes() const;

		std::vector<uint32_t> triangle_mapping;
		Distribution_1D distribution;
	};

	std::vector<Distribution> distributions_;
};

}

#endif
