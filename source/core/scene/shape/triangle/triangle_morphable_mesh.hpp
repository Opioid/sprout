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

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   shape::Intersection& intersection) const final override;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Transformation& transformation,
						  const Ray& ray, const material::Materials& materials,
						  Worker& worker, Sampler_filter filter) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float3_p n, float area,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float area, bool two_sided,
						sampler::Sampler& sampler, Node_stack& node_stack,
						Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float2 uv, float area,
						Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float3_p wi, float area,
						Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Transformation& transformation,
					  float3_p p, float3_p wi, float area, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const final override;

	virtual float area(uint32_t part, float3_p scale) const final override;

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

