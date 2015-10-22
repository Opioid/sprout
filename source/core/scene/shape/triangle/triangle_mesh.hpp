#pragma once

#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "bvh/triangle_bvh_data_mt.hpp"
#include "bvh/triangle_bvh_data_yf.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle;

class Mesh : public Shape {
public:

	void init();

	virtual uint32_t num_parts() const final override;

	virtual bool intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   shape::Intersection& intersection) const final override;

	virtual bool intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const final override;

	virtual float opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
						  const math::float2& bounds, Node_stack& node_stack,
						  const material::Materials& materials,
						  const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& n, bool total_sphere,
						sampler::Sampler& sampler, Sample& sample) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, math::float2 uv, Sample& sample) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& wi, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const entity::Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi, bool total_sphere) const final override;

	virtual float area(uint32_t part, const math::float3& scale) const final override;

	virtual bool is_complex() const final override;

	virtual bool is_analytical() const final override;

	virtual void prepare_sampling(uint32_t part, const math::float3& scale) final override;

private:

    typedef bvh::Tree<bvh::Data_MT> Tree;

    Tree tree_;

	struct Distribution {
        void init(uint32_t part, const Tree& tree, const math::float3& scale);
		uint32_t sample(float r);

		math::Distribution_1D distribution;
		std::vector<uint32_t> triangle_mapping;
	};

	std::vector<Distribution> distributions_;

	friend class Provider;
};

}}}

