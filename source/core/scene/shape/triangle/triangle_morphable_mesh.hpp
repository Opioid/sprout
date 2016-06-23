#pragma once

#include "scene/shape/shape.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MT;
class  Morph_target_collection;

class Morphable_mesh : public Shape, public Morphable_shape {
public:

	Morphable_mesh(std::shared_ptr<Morph_target_collection> collection, uint32_t num_parts);

	void init();

	virtual uint32_t num_parts() const final override;

	virtual bool intersect(const Entity_transformation& transformation, math::Oray& ray,
						   Node_stack& node_stack,
						   shape::Intersection& intersection) const final override;

	virtual bool intersect_p(const Entity_transformation& transformation, const math::Oray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Entity_transformation& transformation, const math::Oray& ray,
						  float time, const material::Materials& materials, Worker& worker,
						  material::Sampler_settings::Filter filter) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const float3& p, const float3& n,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const float3& p,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const float3& p,
						float2 uv, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area,  const float3& p,
						const float3& wi, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Entity_transformation& transformation,
					  float area, const float3& p, const float3& wi, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const final override;

	virtual float area(uint32_t part, const float3& scale) const final override;

	virtual bool is_complex() const final override;

	virtual bool is_analytical() const final override;

	virtual void prepare_sampling(uint32_t part) final override;

	virtual Morphable_shape* morphable_shape() final override;

	virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) final override;

private:

	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_MT>>;

	Tree tree_;

	std::shared_ptr<Morph_target_collection> collection_;

	uint32_t num_parts_;

	std::vector<Vertex> vertices_;

	friend class Provider;
};

}}}

