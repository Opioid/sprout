#pragma once

#include "scene/entity/entity.hpp"
#include "scene/material/material.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace thread { class Pool; }

namespace scene {

class Worker;
struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}

class Prop : public entity::Entity {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	virtual ~Prop();

	void set_shape_and_materials(std::shared_ptr<shape::Shape> shape,
								 const material::Materials& materials);

	virtual void set_parameters(const json::Value& parameters) final override;

	void prepare_sampling(uint32_t part, uint32_t light_id,
						  bool material_importance_sampling, thread::Pool& pool);

	void morph(thread::Pool& pool);

	bool intersect(Ray& ray, shape::Node_stack& node_stack,
				   shape::Intersection& intersection) const;

	bool intersect_p(const Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const Ray& ray, Worker& worker, Sampler_filter filter) const;

	float3 absorption(const Ray& ray, Worker& worker, Sampler_filter filter) const;

	const shape::Shape* shape() const;
	shape::Shape* shape();

	const math::aabb& aabb() const;

	float area(uint32_t part) const;
	uint32_t light_id(uint32_t part) const;
	material::Material* material(uint32_t part) const;

	bool has_masked_material() const;
	bool has_translucent_shadow() const;

	bool is_open() const;
	void set_open(bool open);

private:

	bool visible(uint32_t ray_depth) const;

	virtual void on_set_transformation() final override;

	// Pre-transformed AABB in world space.
	// For moving objects it must cover the entire area occupied by the object during the tick.
	math::aabb aabb_;

	std::shared_ptr<shape::Shape> shape_;

	struct Part {
		float    area;
		uint32_t light_id;
	};

	std::vector<Part> parts_;

	material::Materials materials_;
};

}
